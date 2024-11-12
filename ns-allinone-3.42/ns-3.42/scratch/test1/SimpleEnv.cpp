#include <ns3/ai-module.h>
#include <ns3/core-module.h>
#include <iostream>

namespace ns3 {

class SimpleEnv : public OpenGymEnv {
public:
    SimpleEnv();
    ~SimpleEnv() override;
    static TypeId GetTypeId();
    void DoDispose() override;

    Ptr<OpenGymSpace> GetActionSpace() override;
    Ptr<OpenGymSpace> GetObservationSpace() override;
    bool GetGameOver() override;
    Ptr<OpenGymDataContainer> GetObservation() override;
    float GetReward() override;
    std::string GetExtraInfo() override;
    bool ExecuteActions(Ptr<OpenGymDataContainer> action) override;

private:
    std::vector<float> features;  // Predefined list of features
    bool isAttackDetected;
};

SimpleEnv::SimpleEnv() {
    SetOpenGymInterface(OpenGymInterface::Get());
    features = {0.3, 0.7, 0.5, 0.2};  // Example features (e.g., drop rate, hop count, etc.)
    isAttackDetected = false;
}

SimpleEnv::~SimpleEnv() {}

TypeId SimpleEnv::GetTypeId() {
    static TypeId tid = TypeId("ns3::SimpleEnv").SetParent<OpenGymEnv>().SetGroupName("OpenGym");
    return tid;
}

void SimpleEnv::DoDispose() {}

Ptr<OpenGymSpace> SimpleEnv::GetActionSpace() {
    std::vector<uint32_t> shape = {1};
    Ptr<OpenGymBoxSpace> box = CreateObject<OpenGymBoxSpace>(0, 1, shape, "bool");
    return box;
}

Ptr<OpenGymSpace> SimpleEnv::GetObservationSpace() {
    std::vector<uint32_t> shape = {4};  // 4 features in this example
    Ptr<OpenGymBoxSpace> box = CreateObject<OpenGymBoxSpace>(0.0, 1.0, shape, "float");
    return box;
}

Ptr<OpenGymDataContainer> SimpleEnv::GetObservation() {
    std::vector<uint32_t> shape = {4};
    Ptr<OpenGymBoxContainer<float>> box = CreateObject<OpenGymBoxContainer<float>>(shape);

    for (float feature : features) {
        box->AddValue(feature);
    }

    return box;
}

// New implementation for GetGameOver (return false to keep the simulation running indefinitely)
bool SimpleEnv::GetGameOver() {
    return false;  // Simulation continues until manually stopped
}

// New implementation for GetReward (return 0.0 as no reward is calculated in this example)
float SimpleEnv::GetReward() {
    return 0.0;  // Placeholder reward value
}

// New implementation for GetExtraInfo (return an empty string as no extra info is provided)
std::string SimpleEnv::GetExtraInfo() {
    return "";  // No additional information
}

bool SimpleEnv::ExecuteActions(Ptr<OpenGymDataContainer> action) {
    Ptr<OpenGymBoxContainer<bool>> box = DynamicCast<OpenGymBoxContainer<bool>>(action);
    isAttackDetected = box->GetValue(0);

    if (isAttackDetected) {
        std::cout << "Potential attack detected based on predefined features." << std::endl;
    } else {
        std::cout << "No attack detected." << std::endl;
    }
    return true;
}

}  // namespace ns3

int main(int argc, char *argv[]) {
    ns3::Ptr<ns3::SimpleEnv> simpleEnv = ns3::CreateObject<ns3::SimpleEnv>();
    simpleEnv->NotifySimulationEnd();
    return 0;
}
