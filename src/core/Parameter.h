#pragma once
#include <array>
#include <cstdint>

namespace ss {
enum class Parameter : std::uint8_t {
    Note = 0,
    Instrument,
    Volume,
    Pan,
    Pitch,
    Cutoff,
    Resonance,
    Chance,
    Length,
    Retrigger,
    Delay,
    Count
};
constexpr std::size_t PARAMETER_COUNT = static_cast<std::size_t>(Parameter::Count);

struct ParameterValue {
    bool set = false;
    std::uint8_t value = 0;
    static constexpr ParameterValue Unset() { return {}; }
    static constexpr ParameterValue Set(std::uint8_t v) { return {true, v}; }
};

const char* parameterName(Parameter p);
}
