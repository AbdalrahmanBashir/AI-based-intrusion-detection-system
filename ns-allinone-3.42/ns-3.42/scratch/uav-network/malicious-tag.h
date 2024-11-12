#ifndef MALICIOUS_TAG_H
#define MALICIOUS_TAG_H

#include "ns3/object.h"

namespace ns3 {

class MaliciousTag : public Object {
public:
    static TypeId GetTypeId(void);
    MaliciousTag();
    ~MaliciousTag();
    bool IsMalicious() const; // Check if the node is malicious

private:
    bool m_isMalicious = true; // Default to true since it's a malicious tag
};

} // namespace ns3

#endif // MALICIOUS_TAG_H
