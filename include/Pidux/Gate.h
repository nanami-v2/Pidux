// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nanami-v2
#pragma once
#include <mutex>
#include <boost/container/static_vector.hpp>

namespace Pidux {

class Gate {
public:
    class Callback {
    public:
        virtual ~Callback() noexcept = default;
        virtual void onUnlocked() noexcept = 0;
    };
public:
    Gate() noexcept = default;
    Gate(Gate const&) = delete;
    Gate(Gate&&) noexcept = delete;
    ~Gate() noexcept = default;

    Gate& operator=(Gate const&) = delete;
    Gate& operator=(Gate&&) noexcept = delete;

    void registerCallback(Callback& cb);
    void unregisterCallback(Callback& cb);

    void incrementLockCount() noexcept;
    void incrementLockCountAndMax() noexcept;
    void decrementLockCount() noexcept;
    void decrementLockCountAndMax() noexcept;

    void lock() noexcept;
    void unlock() noexcept;    
private:
    struct SharedData {
        unsigned int lockCount{0};
        unsigned int lockMax{0};
    };
    SharedData sharedData_;
    std::mutex sharedDataMutex_;
    boost::container::static_vector<Callback*, 32> callbacks_;
};

/*-----------------------------------------------------------------------------
    Implementation
-----------------------------------------------------------------------------*/

inline void Gate::registerCallback(Callback& cb) {
    auto const ite = std::find(this->callbacks_.begin(), this->callbacks_.end(), &cb);
    auto const found = (ite != this->callbacks_.end());

    if (!found)
        this->callbacks_.push_back(&cb);
}

inline void Gate::unregisterCallback(Callback& cb) {
    auto const ite = std::find(this->callbacks_.begin(), this->callbacks_.end(), &cb);
    auto const found = (ite != this->callbacks_.end());

    if (found)
        this->callbacks_.erase(ite);
}

inline void Gate::incrementLockCount() noexcept {
    std::unique_lock<std::mutex> lock{this->sharedDataMutex_};

    if (this->sharedData_.lockCount < this->sharedData_.lockMax)
        this->sharedData_.lockCount++;
}

inline void Gate::incrementLockCountAndMax() noexcept {
    std::unique_lock<std::mutex> lock{this->sharedDataMutex_};

    this->sharedData_.lockCount++;
    this->sharedData_.lockMax++;
}

inline void Gate::decrementLockCount() noexcept {
    bool unlocked = false;
    /* ロックエリア */
    {
        std::unique_lock<std::mutex> lock{this->sharedDataMutex_};

        this->sharedData_.lockCount = (this->sharedData_.lockCount > 0) ? this->sharedData_.lockCount - 1 : 0;

        if (this->sharedData_.lockCount == 0) {
            this->sharedData_.lockCount = this->sharedData_.lockMax;
            unlocked = true;
        }
    }
    if (unlocked) {
        for (auto* const cb : this->callbacks_)
            cb->onUnlocked();
    }
}

inline void Gate::decrementLockCountAndMax() noexcept {
    bool unlocked = false;
    /* ロックエリア */
    {
        std::unique_lock<std::mutex> lock{this->sharedDataMutex_};

        this->sharedData_.lockCount = (this->sharedData_.lockCount > 0) ? this->sharedData_.lockCount - 1 : 0;
        this->sharedData_.lockMax   = (this->sharedData_.lockMax   > 0) ? this->sharedData_.lockMax   - 1 : 0;

        if (this->sharedData_.lockCount == 0) {
            this->sharedData_.lockCount = this->sharedData_.lockMax;
            unlocked = true;
        }
    }
    if (unlocked) {
        for (auto* const cb : this->callbacks_)
            cb->onUnlocked();
    }
}

inline void Gate::lock() noexcept {
    std::unique_lock<std::mutex> lock{this->sharedDataMutex_};

    this->sharedData_.lockCount = this->sharedData_.lockMax;
}

inline void Gate::unlock() noexcept {
    /* ロックエリア */
    {
        std::unique_lock<std::mutex> lock{this->sharedDataMutex_};

        this->sharedData_.lockCount = this->sharedData_.lockMax;
    }
    for (auto* const cb : this->callbacks_)
        cb->onUnlocked();
}

} /* namespace Pidux */