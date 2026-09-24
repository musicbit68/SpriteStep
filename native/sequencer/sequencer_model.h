#ifndef SPRITESTEP_SEQUENCER_MODEL_H
#define SPRITESTEP_SEQUENCER_MODEL_H

#include "../songcore/model.h"
#include <algorithm>

namespace sequencer {

// The persistent Pattern/Bank/Arrange data now lives in songcore::Project::sequencer.  These aliases
// keep the sequencer implementation/UI readable without maintaining a second project graph.
constexpr int TRACK_COUNT = songcore::SEQUENCER_TRACKS;
constexpr int BANK_COUNT = songcore::SEQUENCER_BANKS;
constexpr int PATTERN_COUNT = songcore::SEQUENCER_PATTERNS;
constexpr int MAX_PATTERN_STEPS = songcore::SEQUENCER_MAX_STEPS;

using PatternStep = songcore::PhraseStep;
using Pattern = songcore::SequencerPattern;
using Bank = songcore::SequencerBank;
using Track = songcore::SequencerTrack;
using PatternRef = songcore::SequencerPatternRef;
using ArrangeScene = songcore::SequencerArrangeScene;
using Project = songcore::SequencerData;

struct TrackCue {
    bool pending = false;
    uint8_t bank = 0;
    uint8_t pattern = 0;
};

// Runtime state is deliberately separate from saved project data and UI state.
struct TrackRuntime {
    bool active = false;
    uint8_t bank = 0;
    uint8_t pattern = 0;
    int step = 0;
    uint64_t pattern_repeat = 0;
    int64_t pattern_start_frame = 0;
    int64_t next_step_frame = 0;
    TrackCue cue{};
};

struct Runtime {
    bool playing = false;
    int scene = 0;
    int64_t scene_start_frame = 0;
    int64_t scene_end_frame = 0;
    std::array<TrackRuntime, TRACK_COUNT> tracks{};
};

inline bool valid_pattern_ref(const PatternRef& ref) {
    return ref.active && ref.bank < BANK_COUNT && ref.pattern < PATTERN_COUNT;
}

inline const Pattern& pattern_at(const Project& project, int track, int bank, int pattern) {
    return project.tracks[static_cast<size_t>(track)]
        .banks[static_cast<size_t>(bank)]
        .patterns[static_cast<size_t>(pattern)];
}

inline Pattern& pattern_at(Project& project, int track, int bank, int pattern) {
    return project.tracks[static_cast<size_t>(track)]
        .banks[static_cast<size_t>(bank)]
        .patterns[static_cast<size_t>(pattern)];
}

inline int step_duration_multiplier(const Track& track) {
    return std::clamp(static_cast<int>(track.step_duration_multiplier), 1, 8);
}

inline int64_t pattern_duration_base_steps(const Pattern& pattern, const Track& track) {
    return static_cast<int64_t>(pattern.clamped_length()) * step_duration_multiplier(track);
}

} // namespace sequencer

#endif
