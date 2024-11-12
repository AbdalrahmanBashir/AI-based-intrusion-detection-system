#ifndef IMPERSONATION_ATTACK_APP_H
#define IMPERSONATION_ATTACK_APP_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/ipv4.h"
#include "ns3/vector.h"
#include "ns3/udp-socket-factory.h"

namespace ns3 {
        struct PDUIm {
        Vector position;
        Vector velocity;
    };

class ImpersonationAttackApp : public Application {
public:
    static TypeId GetTypeId(void);
    ImpersonationAttackApp();
    virtual ~ImpersonationAttackApp();
    void SetPeerPort(uint16_t port);
    void SetImpersonatedNode(Ptr<Node> node);  // Set the legitimate node to impersonate

protected:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

private:
    void ImpersonateAndSend();  // The method for impersonation and sending packets

    Ptr<Socket> m_socket;
    EventId m_impersonationEvent;
    Ptr<Node> m_impersonatedNode;  // Legitimate node to impersonate
    uint16_t m_peerPort;
};

}  // namespace ns3

#endif  // IMPERSONATION_ATTACK_APP_H
