#include "sequencer/TrackPlayer.h"
#include "core/Constants.h"
namespace ss {
std::size_t TrackPlayer::stepAtTransport(Tick transportTick) const {
    if (!pattern_ || pattern_->length == 0) return 0;
    Tick step = transportTick / PPQ;
    switch (pattern_->speed) {
        case SpeedMultiplier::Half: step = transportTick / (PPQ * 2); break;
        case SpeedMultiplier::One: step = transportTick / PPQ; break;
        case SpeedMultiplier::Double: step = (transportTick * 2) / PPQ; break;
        case SpeedMultiplier::Quadruple: step = (transportTick * 4) / PPQ; break;
    }
    const std::size_t len = pattern_->length;
    const std::size_t pos = static_cast<std::size_t>((step % static_cast<Tick>(len) + len) % len);
    switch (pattern_->direction) {
        case PlayDirection::Forward: return pos;
        case PlayDirection::Reverse: return len - 1 - pos;
        case PlayDirection::PingPong: {
            if (len <= 1) return 0;
            // Ping-pong repeats both endpoints:
            // 0,1,...,len-1,len-1,...,1,0,0,...
            // This makes the terminal step audible for a full step before reversing.
            const std::size_t period = len * 2;
            const std::size_t x = static_cast<std::size_t>(step % static_cast<Tick>(period));
            return x < len ? x : period - 1 - x;
        }
    }
    return pos;
}
}
