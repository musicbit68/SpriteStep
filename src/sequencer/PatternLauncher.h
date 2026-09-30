#pragma once
#include "core/Types.h"
#include "project/Track.h"
#include "sequencer/Transport.h"

namespace ss {
enum class LaunchMode { Quantized, Legato };

class PatternLauncher {
public:
    void request(Track& track, BankIndex bank, PatternIndex pattern, LaunchMode mode, const Transport& transport);
    void applyIfDue(Track& track, Transport& transport);
    bool pending() const { return pending_; }
private:
    bool pending_ = false;
    BankIndex bank_ = 0;
    PatternIndex pattern_ = 0;
    LaunchMode mode_ = LaunchMode::Quantized;
    Tick launchTick_ = 0;
};
}
