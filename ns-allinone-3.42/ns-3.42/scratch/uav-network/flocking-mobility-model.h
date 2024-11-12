#ifndef FLOCKING_MOBILITY_MODEL_H
#define FLOCKING_MOBILITY_MODEL_H

#include "ns3/mobility-model.h"
#include "ns3/vector.h"
#include "ns3/ipv4-address.h"
#include <map>                
#include <utility>          

namespace ns3{

class FlockingMobilityModel : public MobilityModel
{
  public:
    static TypeId GetTypeId (void);
    FlockingMobilityModel ();
    
    virtual Vector DoGetPosition (void) const;
    virtual void DoSetPosition (const Vector &position);
    virtual Vector DoGetVelocity (void) const;
    void SetVelocity (const Vector &velocity);
    void SetConstantSpeed(double speed);
    
    void UpdatePosition ();
    void UpdateFromNeighbor(Ipv4Address senderIp, Vector position, Vector velocity, Ipv4Address receiverIp);
    void SetMalicious(bool malicious);
    void GetNearbyFlockmates();
  
  private:
    Vector m_position;
    Vector m_velocity;
    struct NeighborInfo 
    {
      Vector position;
      Vector velocity;
      double lastUpdateTime;
    };
    std::map<Ipv4Address, NeighborInfo> m_neighbors;
    double maxStaleTime;
    double timeInSecond;
    double m_constantSpeed;
    const double m_speedVariation = 5.0;
    bool m_malicious = false;
    
    Vector Separation();
    Vector Cohesion();
    Vector Alignment();
    void RemoveStaleNeighbors();
    void EnsureConstantSpeed();
    void ApplyDistanceBasedForce();
  
};

}
#endif