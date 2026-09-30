#pragma once
#include "core/Constants.h"
#include "project/Instrument.h"
#include "project/Track.h"
#include <array>

namespace ss {
struct Project {
    double bpm = 120.0;
    std::array<Track, TRACK_COUNT> tracks{};
    std::array<Instrument, 128> instruments{};
};
}
