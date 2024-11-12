#include "fanet-udp-app.h"
#include "flocking-mobility-model.h"
#include "flooding-attack-app.h"
#include "fstream"
#include "impersonation-attack-app.h"
#include "malicious-tag.h"
#include "sybil-attack-app.h"
#include "CustomEnv.h"

#include "ns3/aodv-helper.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/gauss-markov-mobility-model.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/netanim-module.h"
#include "ns3/netsimulyzer-module.h"
#include "ns3/network-module.h"
#include "ns3/node.h"
#include "ns3/olsr-helper.h"
#include "ns3/propagation-module.h"
#include "ns3/ssid.h"
#include "ns3/wifi-module.h"

#include <ctime>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>

using namespace ns3;

Ptr<netsimulyzer::LogStream> eventLog; // Log stream for the simulation events
void SetupFlockingMobilityModel(NodeContainer& uavNodes, int numberOfUAVs, double speed, int numberOfMaliciousNode);
void SetupGaussMarkovMobilityModel(NodeContainer& uavNodes, double speed);
void CourseChanged(Ptr<const MobilityModel> model);       // Callback function for course changes
void LogPosition(Ptr<Node> node, std::ofstream& logFile); // Function to log the position of a node
void ScheduleLogging(NodeContainer nodes,
                     std::ofstream& logFile,
                     double interval,
                     double simulationTime);
void LogResults(double packetDeliveryRatio,
                double averageDelay,
                double throughput,
                double totalThroughput,
                const std::string& postfix,
                double numberOfUAVs);
std::string GetTimestampPostfix(); // Function to get the current timestamp in the format
                                   // YYYYMMDD_HHMMSS

int
main(int argc, char* argv[])
{
    CommandLine cmd;
    double speed = 20;         // Speed of the UAV
    double simulationTime = 100; // 43200.0//3600; // Simulation time in second
    int numberOfUAVs = 20;       // Number of UAVs in the simulation
    cmd.AddValue("speed", "Speed of UAVs (m/s)", speed);
    cmd.AddValue("simulationTime", "Simulation time (s)", simulationTime);
    cmd.AddValue("numberOfUAVs", "Number of UAVs", numberOfUAVs);
    cmd.Parse(argc, argv);
    int numberOfMaliciousNode = 2;
    std::string postfix = GetTimestampPostfix();
    NodeContainer uavNodes;        // Container for the UAV nodes
    NodeContainer malicious;       // Container for the blackhole nodes
    NodeContainer not_malicious;   // Container for the non-blackhole nodes
    uavNodes.Create(numberOfUAVs); // Create number of UAV nodes

    // Add the UAV nodes to the container
// Add the UAV nodes to the container (starting from the end for malicious nodes)
    for (int i = 0; i < numberOfUAVs; i++) {
        if ( i > numberOfUAVs - numberOfMaliciousNode) {
            malicious.Add(uavNodes.Get(i));
            //i > numberOfUAVs - numberOfMaliciousNode
        } else {
            not_malicious.Add(uavNodes.Get(i));
        }
    }


    // Add a tag to the blackhole nodes
    for (uint32_t i = 0; i < malicious.GetN(); i++)
    {
        Ptr<MaliciousTag> maliciousTag = CreateObject<MaliciousTag>();
        malicious.Get(i)->AggregateObject(maliciousTag);
    }

    // Network Channel
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211ac); // Using 802.11ac for higher throughput
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode",
                                 StringValue("VhtMcs0"),
                                 "ControlMode",
                                 StringValue("VhtMcs0"));

    // Configure the PHY layer
    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper();
    wifiPhy.Set("TxPowerStart", DoubleValue(10));
    wifiPhy.Set("TxPowerEnd", DoubleValue(10));
    wifiPhy.Set("TxPowerLevels", UintegerValue(1));
    wifiPhy.Set("Antennas", UintegerValue(1));
    wifiPhy.Set("MaxSupportedTxSpatialStreams", UintegerValue(1));
    wifiPhy.Set("MaxSupportedRxSpatialStreams", UintegerValue(1));
    wifiPhy.Set("RxGain", DoubleValue(0));
    wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

    // Configure the channel
    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel");
    wifiPhy.SetChannel(wifiChannel.Create());

    WifiMacHelper wifiMac;
    Ssid ssid = Ssid("fanet");
    wifiMac.SetType("ns3::AdhocWifiMac",
                    "Ssid",
                    SsidValue(ssid),
                    "QosSupported",
                    BooleanValue(true)); // Enable QoS
    NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, uavNodes);

    // Configure the routing protocol
    AodvHelper aodv;
    AodvHelper malicious_aodv;

    InternetStackHelper stack;
    stack.SetRoutingHelper(aodv);
    stack.Install(not_malicious);
    if (malicious.GetN() > 0)
    {
        //malicious_aodv.Set("IsMalicious", BooleanValue(false));
        stack.SetRoutingHelper(malicious_aodv);
        stack.Install(malicious);
    }

    // Assign IP Addresses
    Ipv4AddressHelper ipAddrs;
    ipAddrs.SetBase("10.10.0.0", "255.255.255.0");
    Ipv4InterfaceContainer cInterfaces = ipAddrs.Assign(devices);

    // Set up the FlockingMobilityModel with constant speed for all nodes
    SetupFlockingMobilityModel(uavNodes, numberOfUAVs, speed, numberOfMaliciousNode);

    // Set up UDP Applications (FanetUdpApp)
    uint16_t port = 9;
    for (uint32_t i = 0; i < not_malicious.GetN(); ++i)
    {
        Ptr<FanetUdpApp> app = CreateObject<FanetUdpApp>();
        app->SetPeerPort(port);
        app->SetStartTime(Seconds(1));
        app->SetStopTime(Seconds(simulationTime));
        not_malicious.Get(i)->AddApplication(app);
    }


    // Set up Sybil Attack Applications on malicious nodes
   for (uint32_t i = 0; i < malicious.GetN(); ++i) {
        Ptr<SybilAttackApp> sybilApp = CreateObject<SybilAttackApp>();
        sybilApp->SetPeerPort(9);  // Set destination port
        sybilApp->SetNumberOfIdentities(10);  // Configure number of fake identities per Sybil node
        sybilApp->SetStartTime(Seconds(5));  // Start after 5 seconds
        sybilApp->SetStopTime(Seconds(simulationTime-5));
        malicious.Get(i)->AddApplication(sybilApp);
    }

    Ptr<CustomEnv> customEnv = CreateObject<CustomEnv>();


    // Logging
    std::ofstream logFile;
    logFile.open("Pathfinding/aodv-b-uavs-positions-" + postfix + "-" +
                 std::to_string(numberOfUAVs) + ".csv");
    ScheduleLogging(uavNodes, logFile, 1.0, simulationTime);
    // wifiPhy.EnablePcapAll("Wireshark/wireshark-" + postfix);

    // NetSimulyzer configuration
    std::string outputFileName = "visualization/aodv-b-uav-netsimulyzer3d-" + postfix + "-" +
                                 std::to_string(numberOfUAVs) + ".json";
    auto orchestrator = CreateObject<netsimulyzer::Orchestrator>(outputFileName);
    auto decoration = CreateObject<netsimulyzer::Decoration>(orchestrator);
    decoration->SetAttribute("Model", StringValue(netsimulyzer::models::SINGLE_BOARD_COMPUTER));

    netsimulyzer::NodeConfigurationHelper nodeConfigHelper(orchestrator);
    nodeConfigHelper.Set("EnableMotionTrail", BooleanValue(true));
    nodeConfigHelper.Set("Model", StringValue(netsimulyzer::models::QUADCOPTER_UAV));
    nodeConfigHelper.Install(uavNodes);

    auto possibleNodeLocations =
        CreateObject<netsimulyzer::RectangularArea>(orchestrator, Rectangle{-100, 100, -100, 100});
    possibleNodeLocations->SetAttribute("Name", StringValue("Possible Node Locations"));
    possibleNodeLocations->SetAttribute("FillColor", netsimulyzer::Color3Value{204u, 255u, 204u});

    auto infoLog = CreateObject<netsimulyzer::LogStream>(orchestrator);
    eventLog = CreateObject<netsimulyzer::LogStream>(orchestrator);
    *infoLog << "----- UAV Simulation 3D Settings -----\n";
    *infoLog << "Simulation Duration: " << simulationTime << " seconds\n";

    for (auto iter = NodeList::Begin(); iter != NodeList::End(); iter++)
    {
        auto m = (*iter)->GetObject<FlockingMobilityModel>();
        if (!m)
            continue;

        m->TraceConnectWithoutContext("CourseChange", MakeCallback(&CourseChanged));
    }

    // Flow Monitor
    FlowMonitorHelper flowmonHelper;
    Ptr<FlowMonitor> monitor = flowmonHelper.InstallAll();

    Simulator::Stop(Seconds(simulationTime));
    Simulator::Run();

    // NetAnim configuration
    AnimationInterface anim("NetAnim/aodv-b-uav-netanim-" + postfix + "-" +
                            std::to_string(numberOfUAVs) + ".xml");

    // Open CSV file
    std::ofstream outFile("FlowDataNormal/aodv-b-flow-data-normal-" + postfix + "-" +
                          std::to_string(numberOfUAVs) + ".csv");
    outFile << "FlowID,FlowDuration/"
               "s,SrcAddr,SrcPort,DstAddr,DstPort,Protocol,TxPackets,RxPackets,LostPackets,"
            << "TxBytes,RxBytes,TxPacketRate/s,RxPacketRate/s,TxByteRate/s,RxByteRate/s,MeanDelay/"
               "s,MeanJitter/s,"
            << "Throughput/Kbps,MeanPacketSize,PacketDropRate,"
            << "AverageHopCount,label\n";

    // Calculate performance metrics
    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowmonHelper.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
    double totalPacketsSent = 0;
    double totalPacketsReceived = 0;
    double totalDelay = 0;
    double totalThroughput = 0;
    int numFlows = 0;
    std::vector<float> features(19, 0.0);
    // List of known fake IP addresses used by SybilAttackApp
        std::set<Ipv4Address> fakeIpAddresses = {
            Ipv4Address("192.168.0.100"),
            Ipv4Address("192.168.0.101"),
            Ipv4Address("192.168.0.102"),
            Ipv4Address("192.168.0.103"),
            Ipv4Address("192.168.0.104"),
            Ipv4Address("192.168.0.105"),
            Ipv4Address("192.168.0.106"),
            Ipv4Address("192.168.0.107"),
            Ipv4Address("192.168.0.108"),
            Ipv4Address("192.168.0.109"),
        };

    for (const auto& flow : stats)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(flow.first);
        const FlowMonitor::FlowStats& flowStats = flow.second;
        double meanDelay = 0.0;
        double meanJitter = 0.0;
        double throughput = 0.0;
        std::string label;

        if (flowStats.rxPackets > 0)
        {
            meanDelay =
                flowStats.delaySum.GetSeconds() / flowStats.rxPackets; // Mean delay in seconds
            meanJitter =
                flowStats.jitterSum.GetSeconds() / flowStats.rxPackets; // Mean jitter in seconds
        }

        double flowDuration =
            flowStats.timeLastRxPacket.GetSeconds() - flowStats.timeFirstTxPacket.GetSeconds();
        if (flowDuration <= 0)
        {
            flowDuration = 1.0; // Avoid division by zero
        }

        throughput = (flowStats.rxBytes * 8.0) / (1000.0 * flowDuration); // Throughput in kbps

        // Derived Features
        double txPacketRate = flowStats.txPackets / flowDuration; // Packets per second
        double rxPacketRate = flowStats.rxPackets / flowDuration; // Packets per second
        double txByteRate = flowStats.txBytes / flowDuration;     // Bytes per second
        double rxByteRate = flowStats.rxBytes / flowDuration;     // Bytes per second
        double meanPacketSize = (flowStats.txPackets > 0)
                                    ? (flowStats.txBytes / flowStats.txPackets)
                                    : 0.0; // Mean packet size
        double packetDropRate =
            (flowStats.txPackets > 0)
                ? (flowStats.lostPackets / static_cast<double>(flowStats.txPackets))
                : 0.0;
        double avgHopCount =
            (flowStats.rxPackets > 0)
                ? (flowStats.timesForwarded / static_cast<double>(flowStats.rxPackets))
                : 0.0;

        // Determine the label based on source port and malicious tag
        if (t.sourcePort == 9) {

            bool isSybilAttack = false;
            if (fakeIpAddresses.find(t.sourceAddress) != fakeIpAddresses.end()) {
                isSybilAttack = true;
            }

            if (isSybilAttack) {
                label = "Sybil Attack";
            } else {
                label = "Normal Traffic";
            }

            // Output flow statistics
                outFile << flow.first << ","                         // FlowID
                << flowDuration << ","                       // Flow Duration
                << t.sourceAddress << ","                    // Source Address
                << t.sourcePort << ","                       // Source Port
                << t.destinationAddress << ","               // Destination Address
                << t.destinationPort << ","                  // Destination Port
                << (t.protocol == 17 ? "UDP" : "TCP") << "," // Protocol
                << flowStats.txPackets << ","                // Transmitted Packets
                << flowStats.rxPackets << ","                // Received Packets
                << flowStats.lostPackets << ","              // Lost Packets
                << flowStats.txBytes << ","                  // Transmitted Bytes
                << flowStats.rxBytes << ","                  // Received Bytes
                << txPacketRate << ","                       // TX Packet Rate
                << rxPacketRate << ","                       // RX Packet Rate
                << txByteRate << ","                         // TX Byte Rate
                << rxByteRate << ","                         // RX Byte Rate
                << meanDelay << ","                          // Mean Delay
                << meanJitter << ","                         // Mean Jitter
                << throughput << ","                         // Throughput
                << meanPacketSize << ","                     // Mean Packet Size
                << packetDropRate << "," // Packet Drop Rate
                << avgHopCount << ","    // Average Hop Count
                << label << "\n";

            // Accumulate statistics for overall analysis
            totalPacketsSent += flowStats.txPackets;
            totalPacketsReceived += flowStats.rxPackets;
            totalDelay += flowStats.delaySum.GetSeconds();
            totalThroughput += throughput;
            numFlows++;
            // Extract features from flow statistics
        }

        features[0] = flowDuration; // Flow Duration
        features[1] = t.sourcePort;
        features[2] = t.destinationPort;
        features[3] = 0.0;
        features[4] = flowStats.txPackets; // TX Packets
        features[5] = flowStats.rxPackets; // RX Packets
        features[6] = flowStats.lostPackets; // Lost Packets
        features[7] = flowStats.txBytes; // TX Bytes
        features[8] = flowStats.rxBytes; // RX Bytes
        features[9] = txPacketRate; // TX Packet Rate
        features[10] = rxPacketRate; // RX Packet Rate
        features[11] = txByteRate; // TX Byte Rate
        features[12] = rxByteRate; // RX Byte Rate
        features[13] = meanDelay; // Mean Delay
        features[14] = meanJitter; // Mean Jitter
        features[15] = throughput; // Throughput
        features[16] = meanPacketSize; // Mean Packet Size
        features[17] = packetDropRate; // Packet Drop Rate
        features[18] = avgHopCount; // Average Hop Count

        customEnv->UpdateFlowMetrics(features);
    }

                
    double packetDeliveryRatio = (totalPacketsReceived / totalPacketsSent) * 100;
    double averageDelay = (totalPacketsReceived > 0) ? (totalDelay / totalPacketsReceived) : 0;
    double throughput_average = totalThroughput / numFlows;

    outFile.close();
    *infoLog << "Packet Delivery Ratio: " << packetDeliveryRatio << "%\n";
    *infoLog << "Average End-to-End Delay: " << averageDelay << " seconds\n";
    *infoLog << "Average Throughput: " << throughput_average << " Kbps\n";
    *infoLog << "Total Throughput: " << totalThroughput << " Kbps\n";

    // Log simulation results to CSV
    LogResults(packetDeliveryRatio,
               averageDelay,
               throughput_average,
               totalThroughput,
               postfix,
               numberOfUAVs);
    monitor->SerializeToXmlFile("FlowMonitor/aodv-b-flow-data-normal-" + postfix + "-" +
                                    std::to_string(numberOfUAVs) + ".xml",
                                true,
                                true);

   // customEnv->NotifySimulationEnd();
    Simulator::Destroy();
    return 0;
}

void
SetupFlockingMobilityModel(NodeContainer& uavNodes, int numberOfUAVs, double speed, int numberOfMaliciousNode)
{
    for (int i = 0; i < numberOfUAVs; ++i)
    {
        Ptr<FlockingMobilityModel> flockingMobility = CreateObject<FlockingMobilityModel>();
        Ptr<UniformRandomVariable> random_position = CreateObject<UniformRandomVariable>();
        double x = random_position->GetValue(0, 100.0);
        double y = random_position->GetValue(0, 100.0);
        double z = random_position->GetValue(0, 0.0);

        flockingMobility->SetPosition(Vector(x, y, z));
        flockingMobility->SetVelocity(Vector(1.0, 1.0, 1.0));
        flockingMobility->SetConstantSpeed(speed);
        uavNodes.Get(i)->AggregateObject(flockingMobility);

        if (i > numberOfUAVs - numberOfMaliciousNode)
        {
            flockingMobility->SetMalicious(true);
        }
        else
        {
            flockingMobility->SetMalicious(false);
        }
    }
}

void
SetupGaussMarkovMobilityModel(NodeContainer& uavNodes, double speed)
{
    MobilityHelper mobility;
    double velocity = 1.0;

    // Configure GaussMarkovMobilityModel
    mobility.SetMobilityModel(
        "ns3::GaussMarkovMobilityModel",
        "Bounds",
        BoxValue(Box(0, 100000, 0, 100, 0, 100)), // Define simulation bounds
        "TimeStep",
        TimeValue(Seconds(1.0)),
        "Alpha",
        DoubleValue(1.0), // Use 1.0 to fully rely on previous state
        "MeanVelocity",
        StringValue("ns3::ConstantRandomVariable[Constant=" + std::to_string(velocity) +
                    "]"), // Constant speed
        "NormalVelocity",
        StringValue("ns3::NormalRandomVariable[Mean=0.0|Variance=0.0]"), // No variation in speed
        "MeanDirection",
        StringValue("ns3::UniformRandomVariable[Min=0.0|Max=0]"),
        "NormalDirection",
        StringValue("ns3::NormalRandomVariable[Mean=0.0|Variance=0.1]"),
        "MeanPitch",
        StringValue("ns3::ConstantRandomVariable[Constant=0.0]"),
        "NormalPitch",
        StringValue("ns3::NormalRandomVariable[Mean=0.0|Variance=0.1]"),
        "ConstantSpeed",
        DoubleValue(speed));

    // Install mobility model
    mobility.Install(uavNodes);

    // Set initial position
    MobilityHelper positionAllocator;
    positionAllocator.SetPositionAllocator("ns3::RandomBoxPositionAllocator",
                                           "X",
                                           StringValue("ns3::UniformRandomVariable[Min=0|Max=100]"),
                                           "Y",
                                           StringValue("ns3::UniformRandomVariable[Min=0|Max=100]"),
                                           "Z",
                                           StringValue("ns3::UniformRandomVariable[Min=0|Max=0]"));

    positionAllocator.Install(uavNodes);
}

void
CourseChanged(Ptr<const MobilityModel> model)
{
    const auto nodeId = model->GetObject<Node>()->GetId();
    const auto position = model->GetPosition();
    const auto velocity = model->GetVelocity();

    *eventLog << Simulator::Now().GetMilliSeconds() << ": Node [" << nodeId
              << "] Course Change Position: [" << position.x << ", " << position.y << ", "
              << position.z << "] "
              << "Velocity [" << velocity.x << ", " << velocity.y << ", " << velocity.z << "]\n";
}

void
LogPosition(Ptr<Node> node, std::ofstream& logFile)
{
    Ptr<MobilityModel> mob = node->GetObject<MobilityModel>();
    Vector pos = mob->GetPosition();
    logFile << Simulator::Now().GetSeconds() << "," << node->GetId() << "," << pos.x << "," << pos.y
            << "," << pos.z << std::endl;
}

void ScheduleLogging(NodeContainer nodes, std::ofstream& logFile, double interval, double simulationTime) {
    double currentTime = Simulator::Now().GetSeconds();
    if (currentTime + interval <= simulationTime) {
        Simulator::Schedule(Seconds(interval), &LogPosition, nodes.Get(0), std::ref(logFile));
        Simulator::Schedule(Seconds(interval), &ScheduleLogging, nodes, std::ref(logFile), interval, simulationTime);
    }
}


void
LogResults(double packetDeliveryRatio,
           double averageDelay,
           double throughput,
           double totalThroughput,
           const std::string& postfix,
           double numberOfUAVs)
{
    std::ofstream outFile("NetworkMetricsResult/aodv-b-network-metrics-result-" + postfix + "-" +
                          std::to_string(numberOfUAVs) + ".csv");
    outFile << "Metric,Value,Unit\n";
    outFile << "Packet Delivery Ratio," << packetDeliveryRatio << ", " << "%" << "\n";
    outFile << "Average End-to-End Delay," << averageDelay << ", " << " seconds" << "\n";
    outFile << "Average Throughput," << throughput << ", " << " Kbps" << "\n";
    outFile << "Total Throughput," << totalThroughput << ", " << " Kbps" << "\n";
    outFile.close();
}

std::string
GetTimestampPostfix()
{
    std::time_t now = std::time(nullptr);
    std::tm* timeinfo = std::localtime(&now);
    std::stringstream ss;
    ss << std::put_time(timeinfo, "%Y%m%d_%H%M%S");
    return ss.str();
}

