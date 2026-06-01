// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nanami-v2
#pragma once
#include <type_traits>

namespace pidux {

class ExecutionUnit {
public:
    virtual ~ExecutionUnit() noexcept = default;
    virtual void run(void* ctx) = 0;
};


template<typename F>
class ExecutionUnitAdaptor final : public ExecutionUnit {
public:
    explicit ExecutionUnitAdaptor(F f): f_{f}
    {}
    ExecutionUnitAdaptor(ExecutionUnitAdaptor const&) = default;
    ExecutionUnitAdaptor(ExecutionUnitAdaptor&&) noexcept = default;
    ~ExecutionUnitAdaptor() noexcept override = default;

    ExecutionUnitAdaptor& operator=(ExecutionUnitAdaptor const&) = default;
    ExecutionUnitAdaptor& operator=(ExecutionUnitAdaptor&&) noexcept = default;

    void run(void* ctx) override {
        this->f_(ctx);
    }
private:
    F f_;
};

template<typename F>
ExecutionUnitAdaptor<F> makeExecutionUnitAdaptor(F f) {
    static_assert(
        std::is_invocable_v<F, void*>,
        "Error: lambda must be callable with an void* argument"
    );
    return ExecutionUnitAdaptor<F>(f);
}

} /* namespace pidux */