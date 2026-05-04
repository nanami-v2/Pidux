
#include <atomic>
#include <mutex>
#include <bitset>
#include <iostream>
#include <csignal>
#include <Pipeline.h>

namespace {

class TestExecutionUnit final : public Pipeline::ExecutionUnit {
public:
    explicit TestExecutionUnit(char const* name, int lineNo):
        name_{name},
        lineNo_{lineNo}
    {}
    void run(void* ctx) override {
        auto const sleepTime = std::chrono::milliseconds{std::rand() % 500};
        std::cout << name_.c_str();
        std::this_thread::sleep_for(sleepTime);
    }
private:
    std::string name_;
    int         lineNo_;
};

} /* namespace */

int main() {
    /*
        Line1: ---A---|---B---
        Line2: -------|---C---
    */
    Pipeline::Gate ignitionGate{};
    Pipeline::Gate syncGate{};

    Pipeline::ExecutionLine line1{ignitionGate};
    Pipeline::ExecutionLine line2{ignitionGate};

    TestExecutionUnit unitA{"A", 1};
    TestExecutionUnit unitB{"B", 1};
    TestExecutionUnit unitC{"C", 2};

    line1.addExecutionUnit(unitA);
    line1.addGate(syncGate);
    line1.addExecutionUnit(unitB);

    line2.addGate(syncGate);
    line2.addExecutionUnit(unitC);

    line1.buildOn(nullptr);
    line2.buildOn(nullptr);
    ignitionGate.unlock();

    std::cin.get();
    return 0;
}