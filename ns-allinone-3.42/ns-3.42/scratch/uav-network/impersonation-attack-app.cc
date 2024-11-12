#include "impersonation-attack-app.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/inet-socket-address.h"
#include "ns3/simulator.h"
#include "ns3/node-container.h"
#include "ns3/mobility-model.h"
#include "flocking-mobility-model.h"  // For UAV positioning

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ImpersonationAttackApp");

NS_OBJECT_ENSURE_REGISTERED(ImpersonationAttackApp);

TypeId ImpersonationAttackApp::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::ImpersonationAttackApp")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddConstructor<ImpersonationAttackApp>();
    return tid;
}

ImpersonationAttackApp::ImpersonationAttackApp() : m_socket(0), m_peerPort(9) {
}

ImpersonationAttackApp::~ImpersonationAttackApp() {
}

void ImpersonationAttackApp::SetImpersonatedNode(Ptr<Node> node) {
    m_impersonatedNode = node;
}

void ImpersonationAttackApp::StartApplication() {
    NS_LOG_INFO("Impersonation Attack App Start");

    // Set up the socket for the impersonation attack
    m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    InetSocketAddress localAddress = InetSocketAddress(Ipv4Address::GetAny(), m_peerPort);
    m_socket->Bind(localAddress);

    // Start impersonating the legitimate node
    m_impersonationEvent = Simulator::Schedule(Seconds(1.0), &ImpersonationAttackApp::ImpersonateAndSend, this);
}

void ImpersonationAttackApp::StopApplication() {
    NS_LOG_INFO("Impersonation Attack App Stop");

    if (m_socket) {
        m_socket->Close();
        m_socket = 0;
    }

    Simulator::Cancel(m_impersonationEvent);  // Stop the impersonation event
}

// Implement the SetPeerPort method
void ImpersonationAttackApp::SetPeerPort(uint16_t port) {
    m_peerPort = port;
}

void ImpersonationAttackApp::ImpersonateAndSend() {
    if (!m_impersonatedNode) {
        NS_LOG_ERROR("No legitimate node set for impersonation");
        return;
    }

    // Use the impersonated node's position and velocity to create the PDU
    Ptr<FlockingMobilityModel> mobility = m_impersonatedNode->GetObject<FlockingMobilityModel>();
    PDUIm impersonatedPdu;
    impersonatedPdu.position = mobility->GetPosition();   // Impersonated node's position
    impersonatedPdu.velocity = mobility->GetVelocity();   // Impersonated node's velocity

    Ptr<Packet> packet = Create<Packet>((uint8_t*)&impersonatedPdu, sizeof(PDUIm));

    // Send this packet as if it came from the impersonated node
    NodeContainer nodes = NodeContainer::GetGlobal();
    double radius = 100.0;  // Communication radius

    for (NodeContainer::Iterator it = nodes.Begin(); it != nodes.End(); ++it) {
        Ptr<Node> node = *it;

        // Skip the impersonated node itself and the malicious node
        if (node->GetId() == m_impersonatedNode->GetId() || node->GetId() == GetNode()->GetId()) {
            continue;
        }

        Ptr<FlockingMobilityModel> otherMobility = node->GetObject<FlockingMobilityModel>();
        if (!otherMobility) {
            continue;
        }

        Vector otherPosition = otherMobility->GetPosition();
        double distance = (otherPosition - mobility->GetPosition()).GetLength();

        if (distance <= radius) {
            Ipv4Address address = node->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
            m_socket->SendTo(packet, 0, InetSocketAddress(address, m_peerPort));
        }
    }

    // Schedule the next impersonation event
    m_impersonationEvent = Simulator::Schedule(MilliSeconds(1), &ImpersonationAttackApp::ImpersonateAndSend, this);
}

}  // namespace ns3
