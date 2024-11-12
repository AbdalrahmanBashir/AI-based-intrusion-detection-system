#include "malicious-tag.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("MaliciousTag");

NS_OBJECT_ENSURE_REGISTERED(MaliciousTag);

TypeId MaliciousTag::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::MaliciousTag")
                            .SetParent<Object>()
                            .SetGroupName("Network")
                            .AddConstructor<MaliciousTag>();
    return tid;
}

MaliciousTag::MaliciousTag() {
    NS_LOG_FUNCTION(this);
}

MaliciousTag::~MaliciousTag() {
    NS_LOG_FUNCTION(this);
}

bool
MaliciousTag::IsMalicious() const {
    return m_isMalicious;
}

} // namespace ns3
