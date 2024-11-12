#include "sybil-attack-app.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/inet-socket-address.h"
#include "ns3/simulator.h"
#include "ns3/node-container.h"
#include "ns3/mobility-model.h"
#include "flocking-mobility-model.h"
#include "malicious-tag.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SybilAttackApp");

NS_OBJECT_ENSURE_REGISTERED(SybilAttackApp);

TypeId SybilAttackApp::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::SybilAttackApp")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddConstructor<SybilAttackApp>();
    return tid;
}

SybilAttackApp::SybilAttackApp() : m_numIdentities(3), m_peerPort(9) {
}

SybilAttackApp::~SybilAttackApp() {
}

void SybilAttackApp::SetPeerPort(uint16_t port) {
    m_peerPort = port;
}

void SybilAttackApp::SetNumberOfIdentities(uint32_t numIdentities) {
    m_numIdentities = numIdentities;
}

void SybilAttackApp::StartApplication() {
    NS_LOG_INFO("Sybil Attack App Start");

    // Schedule the setup of fake identities
    m_sybilEvent = Simulator::ScheduleNow(&SybilAttackApp::SetupFakeIdentity, this, 0);
}

void SybilAttackApp::StopApplication() {
    NS_LOG_INFO("Sybil Attack App Stop");

    ClearFakeSockets();
    Simulator::Cancel(m_sybilEvent);
}

void SybilAttackApp::SetupFakeIdentity(uint32_t index) {
    if (index >= m_numIdentities) {
        // All fake identities have been set up, schedule the first send event
        m_sybilEvent = Simulator::ScheduleNow(&SybilAttackApp::SendWithFakeIdentity, this);
        return;
    }

    Ptr<Socket> fakeSocket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());

    // Generate a unique source IP for this fake identity
    std::string fakeIpString = "192.168.0." + std::to_string(index + 100);
    Ipv4Address fakeSourceIp = Ipv4Address(fakeIpString.c_str());

    // Bind the socket to the fake IP address
    InetSocketAddress local = InetSocketAddress(fakeSourceIp, m_peerPort);
    fakeSocket->Bind(local);

    m_sockets.push_back(fakeSocket);
    NS_LOG_INFO("Fake identity " << index << " setup with IP: " << fakeIpString);

    // Schedule the next fake identity setup
    m_sybilEvent = Simulator::Schedule(MilliSeconds(50), &SybilAttackApp::SetupFakeIdentity, this, index + 1);
}

void SybilAttackApp::ClearFakeSockets() {
    for (auto socket : m_sockets) {
        socket->Close();
    }
    m_sockets.clear();
}

void SybilAttackApp::SendWithFakeIdentity() {
    for (uint32_t i = 0; i < m_numIdentities; ++i) {
        PDUSy fakePdu;
        fakePdu.position = Vector(100, 50, 80);
        fakePdu.velocity = Vector(2, 2, 2);

        Ptr<Packet> packet = Create<Packet>((uint8_t*)&fakePdu, sizeof(PDUSy));

        NodeContainer nodes = NodeContainer::GetGlobal();
        double radius = 30.0;

        for (NodeContainer::Iterator it = nodes.Begin(); it != nodes.End(); ++it) {
            Ptr<Node> node = *it;

            if (node->GetId() == GetNode()->GetId()) {
                continue;
            }

            if (node->GetObject<MaliciousTag>() != nullptr) {
                continue;
            }

            Ptr<FlockingMobilityModel> otherMobility = node->GetObject<FlockingMobilityModel>();
            if (!otherMobility) {
                continue;
            }

            Vector otherPosition = otherMobility->GetPosition();
            double distance = (otherPosition - fakePdu.position).GetLength();

            if (distance <= radius) {
                Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
                Ipv4Address receiverAddress = ipv4->GetAddress(1, 0).GetLocal();

                m_sockets[i]->SendTo(packet, 0, InetSocketAddress(receiverAddress, m_peerPort));
                NS_LOG_INFO("Fake identity " << i << " sent packet to " << receiverAddress);
            }
        }
    }

    // Schedule the next Sybil packet send
    m_sybilEvent = Simulator::Schedule(MilliSeconds(250), &SybilAttackApp::SendWithFakeIdentity, this);
}

}  // namespace ns3
