#pragma once
#include "core/Parameter.h"
#include <array>

namespace ss {
struct Step {
    std::array<ParameterValue, PARAMETER_COUNT> values{};

    ParameterValue get(Parameter p) const {
        return values[static_cast<std::size_t>(p)];
    }
    void set(Parameter p, ParameterValue v) {
        values[static_cast<std::size_t>(p)] = v;
    }
};
}
