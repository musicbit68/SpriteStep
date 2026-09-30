#pragma once
#include "core/Constants.h"
#include "core/Types.h"

namespace ss {
class Transport {
public:
    explicit Transport(double bpm = 120.0) : bpm_(bpm) {}
    void setBpm(double bpm) { bpm_ = bpm; }
    double bpm() const { return bpm_; }
    void play() { playing_ = true; }
    void stop() { playing_ = false; }
    bool playing() const { return playing_; }
    Tick position() const { return position_; }
    void setPosition(Tick p) { position_ = p; }
    void advance(Tick ticks) { if (playing_) position_ += ticks; }
    Tick ticksPerStep() const { return PPQ; }
    Tick barTicks() const { return PPQ * 4; }
    Tick nextDownbeat() const {
        const Tick bar = barTicks();
        return ((position_ / bar) + 1) * bar;
    }
private:
    double bpm_;
    bool playing_ = false;
    Tick position_ = 0;
};
}
