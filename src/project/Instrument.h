#pragma once
#include "core/Parameter.h"
#include <array>
#include <string>

namespace ss {
struct Instrument {
    std::string name;
    std::array<ParameterValue, PARAMETER_COUNT> defaults{};

    ParameterValue resolve(Parameter p, ParameterValue overrideValue) const {
        return overrideValue.set ? overrideValue : defaults[static_cast<std::size_t>(p)];
    }
};
}
