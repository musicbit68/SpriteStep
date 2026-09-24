#include "banks.h"

#include <algorithm>

#include "../songcore/effects.h"
#include "../songcore/scales.h"

namespace sequencer {
namespace {

uint32_t next_random(uint32_t& state) {
    // Small deterministic generator: no libc/global RNG state, and identical results on ARM/Linux
    // and desktop test builds.
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

songcore::Note random_note(uint32_t& state, unsigned scaleMask, int scaleKey) {
    // Keep generated notes in C3..B5, then snap them to the project's global SCALE. The same
    // scale/key pair is used by Pattern note entry and playback quantization.
    const int midi = 48 + static_cast<int>(next_random(state) % 36);
    int snapped = midi;
    for (int d = 0; d < 12; ++d) {
        const int up = midi + d;
        const int degree = ((up - scaleKey) % 12 + 12) % 12;
        if (up <= 127 && (scaleMask & (1u << degree))) { snapped = up; break; }
        const int down = midi - d;
        const int downDegree = ((down - scaleKey) % 12 + 12) % 12;
        if (d > 0 && down >= 0 && (scaleMask & (1u << downDegree))) { snapped = down; break; }
    }
    return songcore::note_from_midi(snapped);
}

void randomize_step(PatternStep& step, uint32_t& state, unsigned scaleMask, int scaleKey) {
    step = PatternStep{};

    // Choose occupancy FIRST. Empty steps stay genuinely empty so the randomizer produces a pattern
    // of notes rather than eight rows of boxes containing only random default/FX data.
    if ((next_random(state) % 3) == 0) return;

    step.note = random_note(state, scaleMask, scaleKey);
    step.instrument = static_cast<int>(next_random(state) % 8);
    step.volume = static_cast<int>(next_random(state) % 128);

    static constexpr int FX[] = {
        songcore::FX_PAN, songcore::FX_PSL, songcore::FX_CHA,
        songcore::FX_ARPEGGIO, songcore::FX_CCA, songcore::FX_CCB,
        songcore::FX_RSEND, songcore::FX_DSEND
    };

    const int fxCount = static_cast<int>(next_random(state) % 3);
    for (int slot = 1; slot <= fxCount; ++slot) {
        const int code = FX[next_random(state) % (sizeof(FX) / sizeof(FX[0]))];
        songcore::step_set_fx(step, slot, code,
                              static_cast<int>(next_random(state) % 128));
    }
}

void clear_pattern(Pattern& pattern) {
    pattern = Pattern{};
}

} // namespace

void BanksController::select_bank(int bank) {
    bank_ = std::clamp(bank, 0, BANK_COUNT - 1);
}

void BanksController::select_pattern(int pattern) {
    pattern_ = std::clamp(pattern, 0, PATTERN_COUNT - 1);
}

void BanksController::select_track(int track) {
    track_ = std::clamp(track, 0, TRACK_COUNT - 1);
}

Pattern& BanksController::selected_pattern() {
    return pattern_at(project_, track_, bank_, pattern_);
}

const Pattern& BanksController::selected_pattern() const {
    return pattern_at(project_, track_, bank_, pattern_);
}

void BanksController::clear_track_pattern(int track, int bank, int pattern) {
    clear_pattern(pattern_at(project_, track, bank, pattern));
}

void BanksController::randomize_track_pattern(int track, int bank, int pattern, uint32_t seed, unsigned scaleMask, int scaleKey) {
    Pattern& p = pattern_at(project_, track, bank, pattern);
    uint32_t state = seed ? seed : 0x6D2B79F5u;
    for (auto& step : p.steps)
        randomize_step(step, state, scaleMask, scaleKey);
}

void BanksController::clear_all_selected_patterns() {
    for (int track = 0; track < TRACK_COUNT; ++track)
        clear_track_pattern(track, bank_, pattern_);
}

void BanksController::randomize_all_selected_patterns(uint32_t seed, unsigned scaleMask, int scaleKey) {
    uint32_t state = seed ? seed : 0x6D2B79F5u;
    for (int track = 0; track < TRACK_COUNT; ++track) {
        // Give every track a distinct stream while remaining deterministic.
        randomize_track_pattern(track, bank_, pattern_, next_random(state), scaleMask, scaleKey);
    }
}

} // namespace sequencer
