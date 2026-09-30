#pragma once
#include "core/Constants.h"
#include "project/Step.h"
#include <array>
#include <cstdint>

namespace ss {
enum class PlayDirection : std::uint8_t { Forward, Reverse, PingPong };

enum class SpeedMultiplier : std::uint8_t { Half = 0, One = 1, Double = 2, Quadruple = 3 };

struct Pattern {
    std::uint8_t length = 16;
    SpeedMultiplier speed = SpeedMultiplier::One;
    PlayDirection direction = PlayDirection::Forward;
    std::uint8_t shuffle = 0;
    std::array<Step, MAX_STEPS> steps{};

    void setLength(std::uint8_t n);
    Step& step(std::size_t index) { return steps.at(index); }
    const Step& step(std::size_t index) const { return steps.at(index); }
};
}
