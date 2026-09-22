#ifndef SPRITESTEP_SEQUENCER_H
#define SPRITESTEP_SEQUENCER_H

#include <array>
#include <cstdint>
#include <deque>
#include <optional>
#include <vector>

#include "sequencer_model.h"

namespace sequencer {

struct ScheduledStep {
    int track = -1;
    int bank = 0;
    int pattern = 0;
    int step = 0;
    int64_t frame = 0;
    int64_t duration_frames = 0;
    uint64_t pattern_repeat = 0;
    int scene = -1;
    uint8_t wait_ppqn = 0;
    bool trigless = false;
};

class Sequencer {
public:
    explicit Sequencer(Project& project, int sample_rate = 44100);

    void set_tempo(int bpm);
    int tempo() const { return tempo_; }
    int64_t base_step_frames() const;

    void start(int64_t frame = 0);
    void stop();
    bool playing() const { return runtime_.playing; }

    // Begin playback of Arrange scene 0. Each active track starts at the same global frame.
    void start_arrange(int scene = 0, int64_t frame = 0);

    // Fill events until `end_frame`. The caller can send these directly to the SPRITESTEP
    // scheduler/audio adapter. This function never advances the audio clock itself.
    std::vector<ScheduledStep> schedule_until(int64_t end_frame);

    // Queue a Bank/Pattern for one track. If that track is running, the cue is consumed at the
    // end of its current pattern; it does not interrupt the current pattern.
    void cue_pattern(int track, int bank, int pattern);

    // Readback for the UI. This uses the actual scheduled event frame rather than the scheduler's
    // lookahead cursor, so the marker represents what is sounding now rather than what has already
    // been queued for the future.
    std::optional<ScheduledStep> playhead_at(int track, int64_t frame) const;
    int playing_scene_at(int64_t frame) const;
    const TrackCue& cue_state(int track) const;

    const Runtime& runtime() const { return runtime_; }

private:
    int64_t pattern_step_frames(int track) const;
    void enter_scene(int scene, int64_t frame);
    void initialize_track_from_ref(int track, const PatternRef& ref, int64_t frame);
    void emit_next_step(int track, std::vector<ScheduledStep>& out);
    bool condition_passes(const Pattern& pattern, int step, uint64_t patternRepeat) const;
    int authored_step_for(const Track& track, const Pattern& pattern, int ordinal, uint64_t patternRepeat, int trackId) const;
    int64_t shuffle_offset_frames(const Track& track, int ordinal, int64_t stepFrames) const;
    bool advance_scene_if_needed(int64_t frame);

    Project& project_;
    int sample_rate_;
    int tempo_ = 120;
    Runtime runtime_{};
    std::array<std::deque<ScheduledStep>, TRACK_COUNT> scheduled_history_{};
};

} // namespace sequencer

#endif
