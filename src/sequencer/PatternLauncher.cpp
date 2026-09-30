#include "sequencer/PatternLauncher.h"
namespace ss {
void PatternLauncher::request(Track&, BankIndex bank, PatternIndex pattern, LaunchMode mode, const Transport& transport) {
    pending_ = true; bank_ = bank; pattern_ = pattern; mode_ = mode;
    launchTick_ = (mode == LaunchMode::Quantized) ? transport.nextDownbeat() : transport.position() + 1;
}
void PatternLauncher::applyIfDue(Track& track, Transport& transport) {
    if (!pending_ || transport.position() < launchTick_) return;
    track.activeBank = bank_;
    track.activePattern = pattern_;
    pending_ = false;
}
}
