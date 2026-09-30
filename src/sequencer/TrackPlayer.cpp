#include "sequencer/TrackPlayer.h"
#include "core/Constants.h"
namespace ss {
std::size_t TrackPlayer::stepAtTransport(Tick transportTick) const {
    if (!pattern_ || pattern_->length == 0) return 0;
    const Tick step = transportTick / PPQ;
    const std::size_t len = pattern_->length;
    const std::size_t pos = static_cast<std::size_t>((step % static_cast<Tick>(len) + len) % len);
    switch (pattern_->direction) {
        case PlayDirection::Forward: return pos;
        case PlayDirection::Reverse: return len - 1 - pos;
        case PlayDirection::PingPong: {
            if (len <= 1) return 0;
            const std::size_t period = (len - 1) * 2;
            const std::size_t x = static_cast<std::size_t>((step % static_cast<Tick>(period) + period) % period);
            return x < len ? x : period - x;
        }
    }
    return pos;
}
}
