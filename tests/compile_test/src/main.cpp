
#include <atomic>
#include <mutex>
#include <bitset>
#include <iostream>
#include <csignal>
#include <pidux.h>

namespace {

class TestExecutionUnit final : public pidux::ExecutionUnit {
public:
    explicit TestExecutionUnit(char const* name):
        name_{name}
    {}
    void run(void* ctx) override {
        auto const sleepTime = std::chrono::milliseconds{std::rand() % 500};
        std::cout << name_.c_str();
        std::this_thread::sleep_for(sleepTime);
    }
private:
    std::string name_;
};

struct Context {};

} /* namespace */

int main() {
    /*
        Line1: ---A---|---B---
        Line2: -------|---C---
    */
    pidux::Gate ignitionGate{};
    pidux::Gate syncGate{};

    pidux::ExecutionLine line1{ignitionGate};
    pidux::ExecutionLine line2{ignitionGate};

    /* ExcutionUnit: class version */
    TestExecutionUnit unitA{"A"};
    TestExecutionUnit unitB{"B"};

    /* ExcutionUnit: lambda version */
    auto unitC = pidux::makeExecutionUnitAdaptor([](auto* ctx) {
        auto const sleepTime = std::chrono::milliseconds{std::rand() % 500};

        std::cout << "C";
        std::this_thread::sleep_for(sleepTime);
    });

    line1.addExecutionUnit(unitA);
    line1.addGate(syncGate);
    line1.addExecutionUnit(unitB);

    line2.addGate(syncGate);
    line2.addExecutionUnit(unitC);

    Context ctx{};

    line1.buildOn(&ctx);
    line2.buildOn(&ctx);
    ignitionGate.unlock();

    std::cin.get();
    return 0;
}