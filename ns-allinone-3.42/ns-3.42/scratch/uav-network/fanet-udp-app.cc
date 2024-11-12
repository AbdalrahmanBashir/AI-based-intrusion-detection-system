#include "fanet-udp-app.h"

#include "flocking-mobility-model.h"
#include "malicious-tag.h"

#include "ns3/inet-socket-address.h"
#include "ns3/ipv4.h"
#include "ns3/log.h"
#include "ns3/node-container.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/wifi-phy.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("FanetUdpApp");

NS_OBJECT_ENSURE_REGISTERED(FanetUdpApp);

// This function is used to get the TypeId of the FanetUdpApp class.
// The TypeId is a unique identifier for each class in NS-3.
//
// The TypeId is initialized with the string "FanetUdpApp" and set to be
// the parent class of Application. The parent class is set to ensure
// that any methods or attributes defined in the parent class are
// inherited by the child class.
//
// The group name is set to "Applications". This is used to group related
// classes together in the NS-3 module system.
//
// AddConstructor is used to register a constructor for the FanetUdpApp
// class. This constructor is used to create an instance of the class
// when it is needed.
TypeId
FanetUdpApp::GetTypeId(void)
{
    // Static variable to ensure that the TypeId is only initialized once.
    static TypeId tid = TypeId("FanetUdpApp")
                            // Set the parent class to Application.
                            .SetParent<Application>()
                            // Set the group name for the class.
                            .SetGroupName("Applications")
                            // Register the constructor for the class.
                            .AddConstructor<FanetUdpApp>();
    // Return the TypeId.
    return tid;
}

FanetUdpApp::FanetUdpApp()
    : m_socket(0),
      m_peerPort(9),
      broadcastDelay(5)
{
    m_neighbors.clear();
}

void
FanetUdpApp::SetPeerPort(uint16_t port)
{
    m_peerPort = port;
}

/**
 * Start the application.
 *
 * This function performs the following steps:
 * 1. Get the mobility model object from the current node.
 * 2. If the mobility model object is not found, print an error message and exit.
 * 3. Create a socket object for the current node.
 * 4. Bind the socket to the local address (any IP address and the specified port).
 * 5. Set a callback function to handle incoming packets.
 * 6. Generate a random offset between 0.1 and 0.9 seconds.
 * 7. Schedule the execution of the UpdateNeighbors function after the specified delay.
 * 8. Schedule the execution of the SendPacket function after the specified delay.
 */
void
FanetUdpApp::StartApplication()
{
    // Get the mobility model object from the current node
    m_mobility = GetNode()->GetObject<FlockingMobilityModel>();

    // If the mobility model object is not found, print an error message and exit
    if (!m_mobility)
    {
        NS_FATAL_ERROR("MobilityModel not installed on node");
    }

    // Create a socket object for the current node
    m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());

    // Bind the socket to the local address (any IP address and the specified port)
    InetSocketAddress localAddress = InetSocketAddress(Ipv4Address::GetAny(), m_peerPort);
    m_socket->Bind(localAddress);

    // Set a callback function to handle incoming packets
    m_socket->SetRecvCallback(MakeCallback(&FanetUdpApp::ReceivePacket, this));

    // Generate a random offset between 0.1 and 0.9 seconds
    Ptr<UniformRandomVariable> random = CreateObject<UniformRandomVariable>();
    double offset = random->GetValue(0.1, 0.6);
    // Schedule the execution of the SendPacket function after the specified delay
    m_sendEvent =
        Simulator::Schedule(Seconds(broadcastDelay + offset), &FanetUdpApp::SendPacket, this);
}

/**
 * This function is called when the application is stopped.
 * It performs the following steps:
 * 1. If the socket object is not null, close the socket and set it to null.
 * 2. Cancel the scheduled event for the SendPacket function.
 */
void
FanetUdpApp::StopApplication(void)
{
    // Check if the socket object is not null
    if (m_socket)
    {
        // Close the socket and set it to null
        m_socket->Close();
        m_socket = 0;
    }

    // Cancel the scheduled event for the SendPacket function
    Simulator::Cancel(m_sendEvent);
}

// This function is responsible for sending a PDU (Protocol Data Unit) to each of the
// neighbors of the current node. The PDU contains the current position and velocity
// of the node. The function is called repeatedly at a fixed interval specified by
// the 'broadcastDelay' variable.
void
ns3::FanetUdpApp::SendPacket()
{
    // Create a PDU (Protocol Data Unit) object and initialize its position and velocity
    // fields with the current position and velocity of the node

    NodeContainer nodes = NodeContainer::GetGlobal();

    // Set the radius within which a neighbor is considered as such
    double radius = 30.0;

    // Iterate over all nodes
    for (NodeContainer::Iterator it = nodes.Begin(); it != nodes.End(); ++it)
    {
        // Get the node being iterated over
        Ptr<Node> node = *it;

        // Skip the current node (since we don't want it to be its own neighbor)
        if (node->GetId() == GetNode()->GetId())
        {
            continue;
        }

        // Check if the node is malicious
        if (node->GetObject<MaliciousTag>() != nullptr)
        {
            continue; // Skip malicious nodes
        }

        // Get the FlockingMobilityModel object associated with the node
        Ptr<FlockingMobilityModel> other = node->GetObject<FlockingMobilityModel>();

        // Get the position of the other node
        Vector otherPosition = other->GetPosition();

        // Calculate the difference vector between the current node's position and the other node's
        // position
        Vector diff = (otherPosition - m_mobility->GetPosition());
        // Calculate the distance between the current node and the other node
        double distance = diff.GetLength();

        // Get the IPv4 address of the other node
        Ipv4Address address = node->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();

        // If the distance between the current node and the other node is within the radius, add the
        // other node's IP address to the list of neighbors
        if (distance <= radius)
        {
            PDU pdu;
            pdu.position = m_mobility->GetPosition();
            pdu.velocity = m_mobility->GetVelocity();
            Ptr<Packet> packet = Create<Packet>((uint8_t*)&pdu, sizeof(PDU));
            m_socket->SendTo(
                packet,
                0,
                InetSocketAddress(address,
                                  m_peerPort)); // Add the neighbor's IP address to the list
        }
    }

    // Schedule the next execution of this function after the specified delay.
    m_sendEvent = Simulator::Schedule(Seconds(broadcastDelay), &FanetUdpApp::SendPacket, this);
}

/**
 * This function is called when a UDP packet is received on the socket.
 * It is responsible for updating the neighbor table with the sender's information
 * and updating the mobility model with the received packet's position and velocity.
 *
 * @param socket The socket that received the packet.
 */
void
FanetUdpApp::ReceivePacket(Ptr<Socket> socket)
{
    if (!socket)
    {
        NS_LOG_ERROR("Invalid socket received.");
        return;
    }

    Address from;
    Ptr<Packet> packet;
    while ((packet = socket->RecvFrom(from)))
    {
        // Create a PDU object to store the packet data.
        PDU pdu;
        if (packet->GetSize() != sizeof(PDU))
        {
            NS_LOG_WARN("Received packet size mismatch. Expected "
                        << sizeof(PDU) << " bytes, got " << packet->GetSize() << " bytes.");
            continue;
        }
        // Update neighbor table with the sender's information
        InetSocketAddress inetAddr = InetSocketAddress::ConvertFrom(from);
        Ipv4Address senderIpv4 = inetAddr.GetIpv4();

        //std::cout << "Node #" << GetNode()->GetId() << " Received packet from: " << senderIpv4
                 // << " & size of: " << packet->GetSize() << " at " << Simulator::Now().GetSeconds()
                 // << std::endl;
        // Copy the PDU data from the packet
        packet->CopyData(reinterpret_cast<uint8_t*>(&pdu), sizeof(PDU));
        // Update the mobility model with the received PDU
        Ptr<Node> node = socket->GetNode();
        Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
        Ipv4Address receiverIpv4 = ipv4->GetAddress(1, 0).GetLocal();
        // Call the UpdateFromNeighbor function to update the mobility model with the received
        // packet's position and velocity.
        m_mobility->UpdateFromNeighbor(senderIpv4, pdu.position, pdu.velocity, receiverIpv4);
    }
}

} // namespace ns3
