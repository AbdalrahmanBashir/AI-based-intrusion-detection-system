#ifndef SYBIL_ATTACK_APP_H
#define SYBIL_ATTACK_APP_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/ipv4.h"
#include "ns3/vector.h"
#include "ns3/udp-socket-factory.h"
#include "flocking-mobility-model.h"

namespace ns3 {

        struct PDUSy {
        Vector position;
        Vector velocity;
    };

class SybilAttackApp : public Application {
public:
    static TypeId GetTypeId(void);
    SybilAttackApp();
    virtual ~SybilAttackApp();

    void SetPeerPort(uint16_t port);

    void SetNumberOfIdentities(uint32_t numIdentities);  // Set how many fake identities to create

protected:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

private:
    void SendWithFakeIdentity();  // Method to send packets with fake identities
    void StartSybilAttack();
    void SetupFakeIdentity(uint32_t index);
    void ClearFakeSockets();

    Ptr<Socket> m_socket;
    EventId m_sybilEvent;
    uint32_t m_numIdentities;  // Number of fake identities to use
    uint16_t m_peerPort;
    std::vector<Ptr<Socket>> m_sockets;
    Ptr<FlockingMobilityModel> m_mobility;
};

}  // namespace ns3

#endif  // SYBIL_ATTACK_APP_H
