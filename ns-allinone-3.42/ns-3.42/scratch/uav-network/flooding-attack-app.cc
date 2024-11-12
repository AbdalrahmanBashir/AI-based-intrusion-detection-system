#include "flooding-attack-app.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/inet-socket-address.h"
#include "ns3/simulator.h"
#include "ns3/node-container.h"
#include "ns3/mobility-model.h"
#include "flocking-mobility-model.h"  // For UAV positioning
#include "malicious-tag.h"            // Include to check for malicious nodes

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("FloodingAttackApp");

NS_OBJECT_ENSURE_REGISTERED(FloodingAttackApp);

TypeId FloodingAttackApp::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::FloodingAttackApp")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddConstructor<FloodingAttackApp>();
    return tid;
}

FloodingAttackApp::FloodingAttackApp() : m_socket(0), floodInterval(0.001), m_peerPort(9) {
}

FloodingAttackApp::~FloodingAttackApp() {
}

void FloodingAttackApp::StartApplication() {
    NS_LOG_INFO("Flooding Attack App Start");

    // Set up the socket and bind to a local address
    m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    InetSocketAddress localAddress = InetSocketAddress(Ipv4Address::GetAny(), m_peerPort);
    m_socket->Bind(localAddress);

    // Start flooding after a short delay
    m_floodEvent = Simulator::Schedule(Seconds(1.0), &FloodingAttackApp::SendFlood, this);
}

void FloodingAttackApp::StopApplication() {
    NS_LOG_INFO("Flooding Attack App Stop");

    if (m_socket) {
        m_socket->Close();
        m_socket = 0;
    }

    Simulator::Cancel(m_floodEvent);  // Stop the flooding event
}

void FloodingAttackApp::SendFlood() {
    // Create a dummy PDU (similar to Fanet UDP PDU)
    PDUP dummyPdu;
    dummyPdu.position = Vector(5.0, 4.0, 40.0);  // Sending incorrect or garbage position
    dummyPdu.velocity = Vector(5.0, 23.0, 0.0);  // Sending incorrect or garbage velocity

    Ptr<Packet> packet = Create<Packet>((uint8_t*)&dummyPdu, sizeof(PDUP));  // Create a flood packet based on Fanet PDU structure

    // Iterate over all nodes to flood neighbors with dummy packets
    NodeContainer nodes = NodeContainer::GetGlobal();
    double radius = 40.0;  // Communication radius

    for (NodeContainer::Iterator it = nodes.Begin(); it != nodes.End(); ++it) {
        Ptr<Node> node = *it;

        // Skip the current node itself
        if (node->GetId() == GetNode()->GetId()) {
            continue;
        }

        // Check if the node is malicious
        if (node->GetObject<MaliciousTag>() != nullptr) {
            continue;  // Skip malicious nodes
        }

        Ptr<FlockingMobilityModel> otherMobility = node->GetObject<FlockingMobilityModel>();
        if (!otherMobility) {
            continue;
        }

        Vector otherPosition = otherMobility->GetPosition();
        double distance = (otherPosition - GetNode()->GetObject<FlockingMobilityModel>()->GetPosition()).GetLength();

        if (distance <= radius) {
            // Get the IP address of the node within range and flood it with the dummy packet
            Ipv4Address address = node->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
            m_socket->SendTo(packet, 0, InetSocketAddress(address, m_peerPort));
        }
    }

    // Schedule the next flood packet at a very high rate (flooding frequency)
    m_floodEvent = Simulator::Schedule(MilliSeconds(50), &FloodingAttackApp::SendFlood, this);
}

}  // namespace ns3
