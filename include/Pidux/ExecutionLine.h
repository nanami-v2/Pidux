// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nanami-v2
#pragma once
#include <bitset>
#include <exception>
#include <optional>
#include <mutex>
#include <thread>
#include <variant>
#include <boost/container/static_vector.hpp>

#include "./Gate.h"
#include "./ExecutionUnit.h"

namespace Pidux {

class ExecutionLine {
public:
    class Callback {
    public:
        virtual ~Callback() noexcept = default;
        virtual void onStart(void* ctx) = 0;
        virtual void onFinished(void* ctx) noexcept = 0;
        virtual void onError(void* ctx, std::exception_ptr error) noexcept = 0;
        virtual void onExecutionError(void* ctx, ExecutionUnit& executionUnit, std::exception_ptr error) noexcept = 0;
    };
public:
    explicit ExecutionLine(Gate& ignitionGate);
    explicit ExecutionLine(Gate& ignitionGate, Callback& cb);
    ExecutionLine(ExecutionLine const&) = delete;
    ExecutionLine(ExecutionLine&&) noexcept = delete;
    ~ExecutionLine() noexcept;

    ExecutionLine& operator=(ExecutionLine const&) = delete;
    ExecutionLine& operator=(ExecutionLine&&) noexcept = delete;
        
    void setCallback(Callback& cb);
    void addExecutionUnit(ExecutionUnit& executionUnit);
    void addGate(Gate& intermediateGate);

    void buildOn(void* ctx);
    void destroy() noexcept;
private:
    struct SharedData {
        bool            shutdownFlag{false};
        bool            ignitionGateUnlockedFlag{false};
        std::bitset<64> intermediateGateUnlockedFlags{};
    };
    enum class GateType {
        IgnitionGate,
        IntermediateGate
    };
    class GateCallback final : public Gate::Callback {
    public:
        explicit GateCallback(
            GateType                   gateType,
            std::optional<std::size_t> gateIndex,
            SharedData&                sharedData,
            std::condition_variable&   sharedDataCv,
            std::mutex&                sharedDataMutex
        );
        void onUnlocked() noexcept override;
    private:
        GateType                   gateType;
        std::optional<std::size_t> gateIndex;
        SharedData&                sharedData;
        std::condition_variable&   sharedDataCv;
        std::mutex&                sharedDataMutex;
    };

    std::thread             thread_;
    SharedData              sharedData_;
    std::condition_variable sharedDataCv_;
    std::mutex              sharedDataMutex_;
    
    Gate&        ignitionGate_;
    GateCallback ignitionGateCallback_;
    boost::container::static_vector<GateCallback,                        64> intermediateGateCallbacks_;
    boost::container::static_vector<std::variant<ExecutionUnit*, Gate*>, 64> lineElements_;
    Callback* callback_{nullptr};
};

/*-----------------------------------------------------------------------------
    Implementation
-----------------------------------------------------------------------------*/

inline ExecutionLine::GateCallback::GateCallback(
    GateType                   gateType,
    std::optional<std::size_t> gateIndex,
    SharedData&                sharedData,
    std::condition_variable&   sharedDataCv,
    std::mutex&                sharedDataMutex
):
    gateType{gateType},
    gateIndex{gateIndex},
    sharedData{sharedData},
    sharedDataCv{sharedDataCv},
    sharedDataMutex{sharedDataMutex}
{}

inline void ExecutionLine::GateCallback::onUnlocked() noexcept {
    std::unique_lock<std::mutex> lock{sharedDataMutex};

    switch (gateType) {
        case GateType::IgnitionGate:
            sharedData.ignitionGateUnlockedFlag = true;
            break;
        case GateType::IntermediateGate:
            sharedData.intermediateGateUnlockedFlags[gateIndex.value()] = true;
            break;
    }
    sharedDataCv.notify_one();
}

/*-----------------------------------------------------------------------------
    Implementation
-----------------------------------------------------------------------------*/

inline ExecutionLine::ExecutionLine(Gate& ignitionGate):
    ignitionGate_{ignitionGate},
    ignitionGateCallback_{
        GateType::IgnitionGate,
        std::nullopt,
        sharedData_,
        sharedDataCv_,
        sharedDataMutex_
    }
{
    ignitionGate.registerCallback(this->ignitionGateCallback_);
    ignitionGate.incrementLockCountAndMax();
}

inline ExecutionLine::ExecutionLine(Gate& ignitionGate, Callback& cb):
    ignitionGate_{ignitionGate},
    ignitionGateCallback_{
        GateType::IgnitionGate,
        std::nullopt,
        sharedData_,
        sharedDataCv_,
        sharedDataMutex_
    },
    callback_{&cb}
{
    ignitionGate.registerCallback(this->ignitionGateCallback_);
    ignitionGate.incrementLockCountAndMax();
}

inline ExecutionLine::~ExecutionLine() noexcept {
    this->destroy();
}

inline void ExecutionLine::setCallback(Callback& cb) {
    this->callback_ = &cb;
}

inline void ExecutionLine::addExecutionUnit(ExecutionUnit& executionUnit) {
    this->lineElements_.push_back(
        &executionUnit
    );
}

inline void ExecutionLine::addGate(Gate& intermediateGate) {
    this->lineElements_.push_back(
        &intermediateGate
    );
    this->intermediateGateCallbacks_.push_back(
        GateCallback{
            GateType::IntermediateGate,
            std::make_optional(this->intermediateGateCallbacks_.size()),
            this->sharedData_,
            this->sharedDataCv_,
            this->sharedDataMutex_
        }
    );
    intermediateGate.registerCallback(this->intermediateGateCallbacks_.back());
    intermediateGate.incrementLockCountAndMax();
}

inline void ExecutionLine::buildOn(void* ctx) {
    this->thread_ = std::thread{[this, ctx]() {
        /* 開始待ち */
        {
            std::unique_lock<std::mutex> lock{this->sharedDataMutex_};

            this->sharedDataCv_.wait(lock, [this] { 
                return (
                    this->sharedData_.shutdownFlag ||
                    this->sharedData_.ignitionGateUnlockedFlag
                );
            });
            if (this->sharedData_.shutdownFlag)
                return;
        }
        /* 開始 */
        try {
            if (this->callback_)
                this->callback_->onStart(ctx);
        }
        catch (...) {
            if (this->callback_) {
                this->callback_->onError(ctx, std::current_exception());
                this->callback_->onFinished(ctx);
            }
            return;
        }
        /* 定常状態 */
        while (true) {
            std::size_t intermediateGateCursor = 0;
            bool        shutdownFlag           = false;

            for (auto& e : this->lineElements_) {
                if (std::holds_alternative<ExecutionUnit*>(e)) {
                    /* ロックエリア */
                    {
                        std::unique_lock<std::mutex> lock{this->sharedDataMutex_};

                        if (this->sharedData_.shutdownFlag)
                            shutdownFlag = true;
                    }
                    if (shutdownFlag) {
                        if (this->callback_)
                            this->callback_->onFinished(ctx);
                        return;
                    }
                    try {
                        std::get<ExecutionUnit*>(e)->run(ctx);
                    }
                    catch (...) {
                        if (this->callback_) {
                            this->callback_->onExecutionError(ctx, *std::get<ExecutionUnit*>(e), std::current_exception());
                            this->callback_->onFinished(ctx);
                        }
                        return;
                    }
                }
                if (std::holds_alternative<Gate*>(e)) {
                    std::get<Gate*>(e)->decrementLockCount();
                    /* ロックエリア */
                    {
                        std::unique_lock<std::mutex> lock{this->sharedDataMutex_};

                        this->sharedDataCv_.wait(lock, [this, intermediateGateCursor] {
                            return (
                                this->sharedData_.shutdownFlag ||
                                this->sharedData_.intermediateGateUnlockedFlags[intermediateGateCursor]
                            );
                        });
                        if (this->sharedData_.shutdownFlag)
                            shutdownFlag = true;
                        else
                            this->sharedData_.intermediateGateUnlockedFlags[intermediateGateCursor] = false;
                    }
                    if (shutdownFlag) {
                        if (this->callback_)
                            this->callback_->onFinished(ctx);
                        return;
                    }
                    intermediateGateCursor++;
                }
            }
        }
    }};
}

inline void ExecutionLine::destroy() noexcept {
    if (this->thread_.joinable()) {
        {
            std::unique_lock<std::mutex> lock{this->sharedDataMutex_};
            this->sharedData_.shutdownFlag = true;
            this->sharedDataCv_.notify_one();
        }
        this->thread_.join();
    }
}

} /* namespace Pidux */
