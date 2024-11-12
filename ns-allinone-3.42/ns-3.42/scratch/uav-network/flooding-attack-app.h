// flooding-attack-app.h
#ifndef FLOODING_ATTACK_APP_H
#define FLOODING_ATTACK_APP_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/ipv4.h"
#include "ns3/vector.h"
#include "ns3/udp-socket-factory.h"

namespace ns3 {

    struct PDUP {
        Vector position;
        Vector velocity;
    };

class FloodingAttackApp : public Application {
public:
    static TypeId GetTypeId(void);
    FloodingAttackApp();
    virtual ~FloodingAttackApp();

protected:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

private:
    void SendFlood();  // Flooding function

    Ptr<Socket> m_socket;
    EventId m_floodEvent;
    double floodInterval;
    uint16_t m_peerPort;
};

}  // namespace ns3

#endif  // FLOODING_ATTACK_APP_H
