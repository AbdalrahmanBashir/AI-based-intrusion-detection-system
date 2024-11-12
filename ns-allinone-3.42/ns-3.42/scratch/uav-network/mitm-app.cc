// mitm-app.cc
#include "mitm-app.h"
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/inet-socket-address.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("MitmApp");

MitmApp::MitmApp() : m_socket(0), m_targetSocket(0) {}

MitmApp::~MitmApp() {
    m_socket = 0;
}

void MitmApp::SetTarget(Ptr<Socket> targetSocket) {
    m_targetSocket = targetSocket;
}

void MitmApp::StartApplication() {
    NS_LOG_FUNCTION(this);

    if (!m_socket) {
        m_socket = Socket::CreateSocket(GetNode(), TypeId::LookupByName("ns3::UdpSocketFactory"));
        m_socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), 9));  // Bind to port 9
        m_socket->SetRecvCallback(MakeCallback(&MitmApp::ReceivePacket, this));
    }
}

void MitmApp::StopApplication() {
    NS_LOG_FUNCTION(this);

    if (m_socket) {
        m_socket->Close();
        m_socket = 0;
    }
}

void MitmApp::ReceivePacket(Ptr<Socket> socket) {
    NS_LOG_FUNCTION(this << socket);

    Ptr<Packet> packet = socket->Recv(); // Receive the packet
    ModifyPacket(packet);                // Modify the packet if needed

    // Forward to target or drop based on your attack strategy
    if (m_targetSocket) {
        m_targetSocket->Send(packet);    // Forward the modified packet to the target
    }
}

void MitmApp::ModifyPacket(Ptr<Packet> packet) {
    NS_LOG_FUNCTION(this << packet);

    // Modify the packet contents here (e.g., change data, inject malicious content)
    NS_LOG_UNCOND("Packet intercepted and modified by MITM");
}

} // namespace ns3
