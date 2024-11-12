// mitm-app.h
#ifndef MITM_APP_H
#define MITM_APP_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/address.h"
#include "ns3/ptr.h"

namespace ns3 {

class MitmApp : public Application {
public:
    MitmApp();
    virtual ~MitmApp();

    /**
     * Set the target socket for forwarding intercepted packets.
     * @param targetSocket The socket to which intercepted packets will be forwarded.
     */
    void SetTarget(Ptr<Socket> targetSocket);

protected:
    /**
     * This function is called at the start of the application.
     */
    virtual void StartApplication() override;

    /**
     * This function is called at the end of the application.
     */
    virtual void StopApplication() override;

private:
    /**
     * Callback function to handle received packets.
     * @param socket The socket on which the packet was received.
     */
    void ReceivePacket(Ptr<Socket> socket);

    /**
     * Function to modify the contents of the intercepted packet.
     * @param packet The packet to be modified.
     */
    void ModifyPacket(Ptr<Packet> packet);

    Ptr<Socket> m_socket;        // Socket for receiving packets
    Ptr<Socket> m_targetSocket;  // Socket to forward intercepted packets
};

} // namespace ns3

#endif // MITM_APP_H
