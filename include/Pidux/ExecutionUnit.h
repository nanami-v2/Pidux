// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nanami-v2
#pragma once

namespace Pidux {

class ExecutionUnit {
public:
    virtual ~ExecutionUnit() noexcept = default;
    virtual void run(void* ctx) = 0;
};

} /* namespace Pidux */