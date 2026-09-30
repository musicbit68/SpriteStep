#pragma once
#include "core/Parameter.h"
#include "core/Types.h"
#include <array>
namespace ss {
struct NoteEvent {
    Tick tick = 0;
    TrackIndex track = 0;
    std::uint8_t instrument = 0;
    std::uint8_t note = 0;
    std::array<ParameterValue, PARAMETER_COUNT> overrides{};
};
}
