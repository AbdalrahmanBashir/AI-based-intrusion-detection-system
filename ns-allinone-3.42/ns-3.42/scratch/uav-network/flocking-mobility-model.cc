#include "flocking-mobility-model.h"

#include "ns3/core-module.h"
#include "ns3/double.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4.h"
#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/node-list.h"
#include "ns3/node.h"
#include "ns3/simulator.h"
#include "ns3/vector.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <omp.h>
#include <queue>
#include <set>
#include <utility>

#define Grean_Color "\033[0;32m"
#define Reset_Color "\033[0m"
#define Yellow_Color "\033[0;33m"
#define Red_Color "\033[0;31m"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("FlockingMobilityModel");

NS_OBJECT_ENSURE_REGISTERED(FlockingMobilityModel);

TypeId
FlockingMobilityModel::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::FlockingMobilityModel")
                            .SetParent<MobilityModel>()
                            .SetGroupName("Mobility")
                            .AddConstructor<FlockingMobilityModel>();
    return tid;
}

FlockingMobilityModel::FlockingMobilityModel()
{
    maxStaleTime = 10.0;
    timeInSecond = 1.0;
    if (m_malicious)
    {
        Simulator::Schedule(Seconds(timeInSecond),
                            &FlockingMobilityModel::GetNearbyFlockmates,
                            this);
    }
    Simulator::Schedule(Seconds(timeInSecond), &FlockingMobilityModel::UpdatePosition, this);
    Simulator::Schedule(Seconds(maxStaleTime), &FlockingMobilityModel::RemoveStaleNeighbors, this);
}

Vector
FlockingMobilityModel::DoGetPosition(void) const
{
    return m_position;
}

void
FlockingMobilityModel::DoSetPosition(const Vector& position)
{
    m_position = Vector(position.x, position.y, position.z);
}

Vector
FlockingMobilityModel::DoGetVelocity(void) const
{
    return m_velocity;
}

void
FlockingMobilityModel::SetVelocity(const Vector& velocity)
{
    m_velocity = Vector(velocity.x, velocity.y, velocity.z);
}

void
FlockingMobilityModel::SetConstantSpeed(double speed)
{
    m_constantSpeed = speed;
}

void
FlockingMobilityModel::SetMalicious(bool malicious)
{
    m_malicious = malicious;
}

void
FlockingMobilityModel::UpdatePosition()
{
    Vector separation(0.0, 0.0, 0.0);
    Vector cohesion(0.0, 0.0, 0.0);
    Vector alignment(0.0, 0.0, 0.0);
    Vector centerOfMass(0.0, 0.0, 0.0);

    int separationCount = 0;
    int cohesionCount = 0;
    int alignmentCount = 0;

    // Optimized radii for separation, cohesion, and alignment
    double separationRadius = 10.0;
    double cohesionRadius = 20.0;
    double alignmentRadius = 30.0;

    // Optimized weights for the different behaviors
    double separationWeight = 30.9;
    double cohesionWeight = 0.8;
    double alignmentWeight = 10.2;

    for (const auto& neighbor : m_neighbors)
    {
        Vector displacement = neighbor.second.position - m_position;
        double distance = displacement.GetLength();

        // Separation behavior
        if (distance < separationRadius)
        {
            displacement.x /= distance;
            displacement.y /= distance;
            displacement.z /= distance;
            separation.x += displacement.x;
            separation.y += displacement.y;
            separation.z += displacement.z;
            separationCount++;
        }

        // Cohesion behavior
        if (distance < cohesionRadius)
        {
            centerOfMass.x += neighbor.second.position.x;
            centerOfMass.y += neighbor.second.position.y;
            centerOfMass.z += neighbor.second.position.z;
            cohesionCount++;
        }

        // Alignment behavior
        if (distance < alignmentRadius)
        {
            alignment.x += neighbor.second.velocity.x;
            alignment.y += neighbor.second.velocity.y;
            alignment.z += neighbor.second.velocity.z;
            alignmentCount++;
        }
    }

    // Finalize Separation
    if (separationCount > 0)
    {
        separation.x /= separationCount;
        separation.y /= separationCount;
        separation.z /= separationCount;
        // Apply repulsion
        separation.x = -separation.x * separationWeight;
        separation.y = -separation.y * separationWeight;
        separation.z = -separation.z * separationWeight;
    }

    // Finalize Cohesion
    if (cohesionCount > 0)
    {
        centerOfMass.x /= cohesionCount;
        centerOfMass.y /= cohesionCount;
        centerOfMass.z /= cohesionCount;
        cohesion.x = (centerOfMass.x - m_position.x) * cohesionWeight;
        cohesion.y = (centerOfMass.y - m_position.y) * cohesionWeight;
        cohesion.z = (centerOfMass.z - m_position.z) * cohesionWeight;
    }

    // Finalize Alignment
    if (alignmentCount > 0)
    {
        alignment.x /= alignmentCount;
        alignment.y /= alignmentCount;
        alignment.z /= alignmentCount;
        alignment.x *= alignmentWeight;
        alignment.y *= alignmentWeight;
        alignment.z *= alignmentWeight;
    }

    // Add a forward bias to ensure the flock moves forward
    double forwardWeight = 3.0;
    Vector forwardBias(forwardWeight, forwardWeight, forwardWeight);

    // Update the velocity based on the rules
    m_velocity.x += separation.x + cohesion.x + alignment.x + (forwardBias.x);
    m_velocity.y += separation.y + cohesion.y + alignment.y + (forwardBias.y);
    m_velocity.z += separation.z + cohesion.z + alignment.z + (forwardBias.z);

    // Maintain constant speed
    double speed = m_velocity.GetLength();
    if (speed != m_constantSpeed)
    {
        m_velocity.x = (m_velocity.x / speed) * m_constantSpeed;
        m_velocity.y = (m_velocity.y / speed) * m_constantSpeed;
        m_velocity.z = (m_velocity.z / speed) * m_constantSpeed;
    }

    // Handle boundaries
    const double minX = 0.0;                      // ground level is at x = 0
    const double maxX = 300.0; // maximum altitude of 1000 units
    const double minY = -100.0;                   // ground level is at y = 0
    const double maxY = 100.0;                    // maximum altitude of 100 units
    const double minZ = 0.0;
    const double maxZ = 100.0;

    Vector nextPosition = m_position + m_velocity * timeInSecond;

    if (nextPosition.x < minX || nextPosition.x > maxX)
    {
        m_velocity.x = -m_velocity.x; // Reverse the x component of the velocity
        nextPosition.x = std::clamp(nextPosition.x, minX, maxX);
    }

    if (nextPosition.y < minY || nextPosition.y > maxY)
    {
        m_velocity.y = -m_velocity.y; // Reverse the y component of the velocity
        nextPosition.y = std::clamp(nextPosition.y, minY, maxY);
    }

    if (nextPosition.z < minZ || nextPosition.z > maxZ)
    {
        m_velocity.z = -m_velocity.z; // Reverse the z component of the velocity
        nextPosition.z = std::clamp(nextPosition.z, minZ, maxZ);
    }

    // Re-normalize to maintain constant speed after reversing components
    speed = m_velocity.GetLength();
    if (speed != m_constantSpeed)
    {
        m_velocity.x = (m_velocity.x / speed) * m_constantSpeed;
        m_velocity.y = (m_velocity.y / speed) * m_constantSpeed;
        m_velocity.z = (m_velocity.z / speed) * m_constantSpeed;
    }

    // Update the position based on the velocity and time step
    m_position.x += m_velocity.x * timeInSecond;
    m_position.y += m_velocity.y * timeInSecond;
    m_position.z += m_velocity.z * timeInSecond;

    // Notify other components about the position change
    NotifyCourseChange();

    // Schedule the next position update
    Simulator::Schedule(Seconds(timeInSecond), &FlockingMobilityModel::UpdatePosition, this);
}

void
FlockingMobilityModel::UpdateFromNeighbor(Ipv4Address senderIp,
                                          Vector position,
                                          Vector velocity,
                                          Ipv4Address receiverIp)
{
    LogComponentEnable("FlockingMobilityModel", LOG_LEVEL_LOGIC);
    auto it = m_neighbors.find(senderIp);

    if (it != m_neighbors.end())
    {
        double currentTime = Simulator::Now().GetSeconds();
        it->second.position = position;
        it->second.velocity = velocity;
        it->second.lastUpdateTime = currentTime;
        // NS_LOG_INFO(Grean_Color << "Updating existing neighbor: " << senderIp << " at time " <<
        // currentTime << Reset_Color);
    }
    else
    {
        m_neighbors.emplace(senderIp,
                            NeighborInfo{position, velocity, Simulator::Now().GetSeconds()});

        // NS_LOG_INFO(Yellow_Color << "Adding new neighbor: " << senderIp << " at time " <<
        // Simulator::Now().GetSeconds() << Reset_Color);
    }
}

void
FlockingMobilityModel::RemoveStaleNeighbors()
{
    // Remove stale neighbors
    LogComponentEnable("FlockingMobilityModel", LOG_LEVEL_LOGIC);
    double currentTime = Simulator::Now().GetSeconds();
    auto it = m_neighbors.begin();
    while (it != m_neighbors.end())
    {
        // Check if the last update time for this neighbor is more than maxStaleTime.
        if (currentTime - it->second.lastUpdateTime > maxStaleTime)
        {
            // Remove the neighbor from the map if it's too old
            it = m_neighbors.erase(it); // erase() returns the iterator to the next valid element
        }
        else
        {
            ++it; // Move to the next element.
        }
    }

    Simulator::Schedule(Seconds(maxStaleTime), &FlockingMobilityModel::RemoveStaleNeighbors, this);
}

void
ns3::FlockingMobilityModel::GetNearbyFlockmates()
{
    double m_neighborRadius = 100.0;

    for (NodeList::Iterator it = NodeList::Begin(); it != NodeList::End(); ++it)
    {
        Ptr<Node> node = *it;
        Ptr<FlockingMobilityModel> flockmate = node->GetObject<FlockingMobilityModel>();
        if (flockmate && flockmate != this)
        {
            Vector distance = flockmate->DoGetPosition() - this->GetPosition();
            if (distance.GetLength() < m_neighborRadius)
            {
                // Get the IP address of the flockmate
                Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
                Ipv4Address flockmateIp;
                if (ipv4)
                {
                    int32_t interface = ipv4->GetInterfaceForDevice(node->GetDevice(0));
                    if (interface >= 0)
                    {
                        flockmateIp = ipv4->GetAddress(interface, 0).GetLocal();
                    }
                }

                Ptr<Node> thisNode = this->GetObject<Node>();
                Ptr<Ipv4> thisIpv4 = thisNode->GetObject<Ipv4>();
                Ipv4Address thisIp;
                if (thisIpv4)
                {
                    int32_t thisInterface = thisIpv4->GetInterfaceForDevice(thisNode->GetDevice(0));
                    if (thisInterface >= 0)
                    {
                        thisIp = thisIpv4->GetAddress(thisInterface, 0).GetLocal();
                    }
                }

                // Call the UpdateFromNeighbor method with the IP addresses and positions/velocities
                UpdateFromNeighbor(flockmateIp,
                                   flockmate->DoGetPosition(),
                                   flockmate->DoGetVelocity(),
                                   thisIp);
            }
        }
    }

    Simulator::Schedule(Seconds(1.0), &FlockingMobilityModel::GetNearbyFlockmates, this);
}

} // namespace ns3