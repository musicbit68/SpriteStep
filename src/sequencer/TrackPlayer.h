#pragma once
#include "project/Pattern.h"
#include "core/Types.h"

namespace ss {
class TrackPlayer {
public:
    void setPattern(const Pattern* p) { pattern_ = p; }
    const Pattern* pattern() const { return pattern_; }
    std::size_t stepAtTransport(Tick transportTick) const;
private:
    const Pattern* pattern_ = nullptr;
};
}
