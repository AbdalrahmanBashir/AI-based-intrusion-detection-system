#include "ns3/application.h"
#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/wifi-module.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/vector.h"
#include "ns3/socket.h"
#include "ns3/core-module.h"
#include "ns3/yans-wifi-phy.h"
#include "flocking-mobility-model.h"
#include "ns3/gauss-markov-mobility-model.h"

namespace ns3 {

    struct PDU {
        Vector position;
        Vector velocity;
    };

    class FanetUdpApp : public Application {
    public:
        static TypeId GetTypeId(void);
        FanetUdpApp();
        void SetPeerPort(uint16_t port);

    private:
        virtual void StartApplication(void) override;
        virtual void StopApplication(void) override;

        void SendPacket();
        void ReceivePacket(Ptr<Socket> socket);
        void SendToNeighbor(Ptr<Packet> packet, Ipv4Address neighbor);
        void UpdateNeighbors();

        Ptr<Socket> m_socket;
        uint16_t m_peerPort;
        EventId m_sendEvent;
        //Ptr<GaussMarkovMobilityModel> m_mobility;
        Ptr<FlockingMobilityModel> m_mobility;
        double broadcastDelay;
        std::vector<Ipv4Address> m_neighbors;
    };

}
