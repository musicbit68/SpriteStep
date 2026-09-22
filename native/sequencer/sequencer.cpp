#include "sequencer.h"

#include <algorithm>
#include <cassert>

namespace sequencer {

namespace {
int64_t frames_per_base_step(int bpm, int sample_rate) {
    // Same musical definition as SPRITESTEP: one base step is one sixteenth note.
    return static_cast<int64_t>(60000.0 / bpm / 4.0 * sample_rate / 1000.0);
}
}

Sequencer::Sequencer(Project& project, int sample_rate)
    : project_(project), sample_rate_(sample_rate) {}

void Sequencer::set_tempo(int bpm) {
    tempo_ = std::max(1, bpm);
}

int64_t Sequencer::base_step_frames() const {
    return frames_per_base_step(tempo_, sample_rate_);
}

int64_t Sequencer::pattern_step_frames(int track) const {
    return base_step_frames() * step_duration_multiplier(project_.tracks[static_cast<size_t>(track)]);
}

void Sequencer::start(int64_t frame) {
    start_arrange(0, frame);
}

void Sequencer::stop() {
    runtime_.playing = false;
}

void Sequencer::start_arrange(int scene, int64_t frame) {
    if (project_.scenes.empty()) {
        runtime_ = Runtime{};
        return;
    }
    scene = std::clamp(scene, 0, static_cast<int>(project_.scenes.size()) - 1);
    runtime_.playing = true;
    runtime_.scene = scene;
    for (auto& h : scheduled_history_) h.clear();
    enter_scene(scene, frame);
}

void Sequencer::enter_scene(int scene, int64_t frame) {
    runtime_.scene = scene;
    runtime_.scene_start_frame = frame;

    const ArrangeScene& arrange = project_.scenes[static_cast<size_t>(scene)];
    int64_t duration = 0;

    for (int t = 0; t < TRACK_COUNT; ++t) {
        TrackRuntime& rt = runtime_.tracks[static_cast<size_t>(t)];
        const PatternRef& ref = arrange.tracks[static_cast<size_t>(t)];
        if (!valid_pattern_ref(ref)) {
            rt.active = false;
            rt.cue.pending = false;
            continue;
        }

        const bool same_pattern = rt.active && rt.bank == ref.bank && rt.pattern == ref.pattern &&
                                  rt.next_step_frame == frame;
        if (!same_pattern) {
            initialize_track_from_ref(t, ref, frame);
        }
        const Pattern& p = pattern_at(project_, t, ref.bank, ref.pattern);
        duration = std::max(duration, pattern_duration_base_steps(p, project_.tracks[static_cast<size_t>(t)]) * base_step_frames());
    }

    runtime_.scene_end_frame = frame + duration;
}

void Sequencer::initialize_track_from_ref(int track, const PatternRef& ref, int64_t frame) {
    TrackRuntime& rt = runtime_.tracks[static_cast<size_t>(track)];
    rt.active = true;
    rt.bank = ref.bank;
    rt.pattern = ref.pattern;
    rt.step = 0;
    rt.pattern_repeat = 0;
    rt.pattern_start_frame = frame;
    rt.next_step_frame = frame;
    rt.cue.pending = false;
}

void Sequencer::cue_pattern(int track, int bank, int pattern) {
    if (track < 0 || track >= TRACK_COUNT || bank < 0 || bank >= BANK_COUNT ||
        pattern < 0 || pattern >= PATTERN_COUNT) return;

    TrackRuntime& rt = runtime_.tracks[static_cast<size_t>(track)];
    rt.cue.pending = true;
    rt.cue.bank = static_cast<uint8_t>(bank);
    rt.cue.pattern = static_cast<uint8_t>(pattern);

    // A stopped/inactive track has no current boundary. Start the cue immediately if it is not
    // participating in the current scene; this keeps the cue API useful for LIVE-style playback.
    if (runtime_.playing && !rt.active) {
        initialize_track_from_ref(track, PatternRef{true, rt.cue.bank, rt.cue.pattern}, runtime_.scene_start_frame);
        rt.cue.pending = false;
    }
}

bool Sequencer::condition_passes(const Pattern& pattern, int step, uint64_t patternRepeat) const {
    if (step < 0 || step >= MAX_PATTERN_STEPS) return false;
    const uint8_t packed = pattern.conditions[static_cast<size_t>(step)];
    if (packed == 0) return true;

    const int occurrence = (packed >> 4) & 0x0F;
    const int count = packed & 0x0F;
    if (occurrence < 1 || count < 1 || occurrence > count) return false;

    // patternRepeat is zero-based. A 1/3 condition therefore fires on cycles 1, 4, 7...
    // The counter belongs to the pattern runtime, so if the same pattern continues into the next
    // Arrange scene its condition phase continues too. A different pattern resets it.
    const uint64_t cycle = patternRepeat % static_cast<uint64_t>(count);
    return cycle == static_cast<uint64_t>(occurrence - 1);
}

int Sequencer::authored_step_for(const Track& track, const Pattern& pattern, int ordinal,
                                  uint64_t patternRepeat, int trackId) const {
    const int length = pattern.clamped_length();
    const int i = std::clamp(ordinal, 0, length - 1);
    switch (track.direction) {
        case songcore::SequencerDirection::FORWARD:
            return i;
        case songcore::SequencerDirection::REVERSE:
            return length - 1 - i;
        case songcore::SequencerDirection::PINGPONG: {
            if (length <= 1) return 0;
            // Preserve the project's fixed pattern-cycle duration: each cycle still emits exactly
            // `length` events, while successive cycles traverse the opposite half of the ping-pong.
            const int period = length * 2 - 2;
            const int phase = static_cast<int>((patternRepeat * static_cast<uint64_t>(length) + i) % period);
            return phase < length ? phase : period - phase;
        }
        case songcore::SequencerDirection::RANDOM: {
            // Deterministic per track/pattern/cycle/ordinal. This keeps the audio schedule repeatable
            // and makes live-edit rollback safe without introducing another mutable RNG stream.
            uint32_t x = 0x9E3779B9u ^ static_cast<uint32_t>(trackId * 0x45D9F3B);
            x ^= static_cast<uint32_t>(patternRepeat) * 0x85EBCA6Bu;
            x ^= static_cast<uint32_t>(i + 1) * 0x27D4EB2Du;
            x ^= x >> 16; x *= 0x7FEB352Du; x ^= x >> 15; x *= 0x846CA68Bu; x ^= x >> 16;
            return static_cast<int>(x % static_cast<uint32_t>(length));
        }
    }
    return i;
}

int64_t Sequencer::shuffle_offset_frames(const Track& track, int ordinal, int64_t stepFrames) const {
    if (track.shuffle == 0 || (ordinal & 1) == 0) return 0;
    // FMS describes shuffle as delaying every other step by up to half a step. We leave the following
    // even step on the original grid, so the pattern's total cycle duration does not change.
    return (stepFrames / 2) * static_cast<int64_t>(track.shuffle) / 255;
}

void Sequencer::emit_next_step(int track, std::vector<ScheduledStep>& out) {
    TrackRuntime& rt = runtime_.tracks[static_cast<size_t>(track)];
    if (!rt.active) return;

    const Pattern& p = pattern_at(project_, track, rt.bank, rt.pattern);
    const Track& tr = project_.tracks[static_cast<size_t>(track)];
    const int length = p.clamped_length();
    if (rt.step >= length) rt.step = 0;

    const int ordinal = rt.step;
    const int authoredStep = authored_step_for(tr, p, ordinal, rt.pattern_repeat, track);
    const int64_t stepFrames = pattern_step_frames(track);
    const uint8_t waitPpqn = p.wait_ppqn[static_cast<size_t>(authoredStep)];
    // FMS defines Wait in PPQN units relative to the track rate: 3 PPQN = half a step and
    // 6 PPQN = one whole step. This is deliberately applied to the already-expanded track step
    // duration, so a 2x track still gets the same musical subdivision.
    const int64_t waitFrames = static_cast<int64_t>(waitPpqn) * stepFrames / 6;
    const int64_t eventFrame = rt.next_step_frame + shuffle_offset_frames(tr, ordinal, stepFrames) + waitFrames;
    if (condition_passes(p, authoredStep, rt.pattern_repeat)) {
        ScheduledStep scheduled{
            track,
            rt.bank,
            rt.pattern,
            authoredStep,
            eventFrame,
            stepFrames,
            rt.pattern_repeat,
            runtime_.scene,
            waitPpqn,
            p.trigless[static_cast<size_t>(authoredStep)] != 0
        };
        out.push_back(scheduled);
        auto& history = scheduled_history_[static_cast<size_t>(track)];
        history.push_back(scheduled);
        while (history.size() > 128) history.pop_front();
    }

    rt.next_step_frame += stepFrames;
    ++rt.step;

    if (rt.step >= length) {
        rt.step = 0;
        ++rt.pattern_repeat;

        // Banks-page cue: switch only after the current pattern has completed.
        if (rt.cue.pending) {
            rt.bank = rt.cue.bank;
            rt.pattern = rt.cue.pattern;
            rt.cue.pending = false;
            rt.pattern_repeat = 0;
            rt.pattern_start_frame = rt.next_step_frame;
        }
    }
}

bool Sequencer::advance_scene_if_needed(int64_t frame) {
    if (!runtime_.playing) return false;
    if (frame < runtime_.scene_end_frame) return false;
    if (runtime_.scene + 1 >= static_cast<int>(project_.scenes.size())) return false;

    enter_scene(runtime_.scene + 1, runtime_.scene_end_frame);
    return true;
}

std::vector<ScheduledStep> Sequencer::schedule_until(int64_t end_frame) {
    std::vector<ScheduledStep> out;
    if (!runtime_.playing) return out;

    while (true) {
        // Keep the end-frame exclusive: schedule everything before it. A scene transition is
        // performed only when the next musical event actually reaches the current scene boundary.
        // This prevents a call ending exactly at a scene boundary from skipping the old scene's
        // final event and also preserves same-pattern continuity across Arrange scenes.
        int nextTrack = -1;
        int64_t nextFrame = end_frame;
        for (int t = 0; t < TRACK_COUNT; ++t) {
            const TrackRuntime& rt = runtime_.tracks[static_cast<size_t>(t)];
            if (!rt.active) continue;
            if (rt.next_step_frame < nextFrame) {
                nextFrame = rt.next_step_frame;
                nextTrack = t;
            }
        }

        if (nextTrack < 0) break;
        if (nextFrame >= runtime_.scene_end_frame) {
            if (!advance_scene_if_needed(nextFrame)) break;
            continue;
        }

        emit_next_step(nextTrack, out);
    }

    // If the caller's window ends exactly at a scene boundary, advance the runtime state to the
    // next scene now, but leave that scene's first event for the next scheduling window.
    if (runtime_.playing && end_frame >= runtime_.scene_end_frame &&
        runtime_.scene + 1 < static_cast<int>(project_.scenes.size())) {
        enter_scene(runtime_.scene + 1, runtime_.scene_end_frame);
    }

    return out;
}

std::optional<ScheduledStep> Sequencer::playhead_at(int track, int64_t frame) const {
    if (track < 0 || track >= TRACK_COUNT || !runtime_.playing) return std::nullopt;
    const auto& history = scheduled_history_[static_cast<size_t>(track)];
    std::optional<ScheduledStep> result;
    for (const auto& event : history) {
        if (event.frame <= frame && (!result || event.frame >= result->frame)) result = event;
    }
    return result;
}

int Sequencer::playing_scene_at(int64_t frame) const {
    (void)frame;
    // The runtime owns the scene transition. Do not infer the scene from the last scheduled
    // musical event: a scene can contain an empty track/pattern, or there can be a gap after the
    // last event while the scene is still active. ARRANGE therefore needs the authoritative runtime
    // scene rather than an event-history guess.
    return runtime_.playing ? runtime_.scene : -1;
}

const TrackCue& Sequencer::cue_state(int track) const {
    static const TrackCue empty{};
    if (track < 0 || track >= TRACK_COUNT) return empty;
    return runtime_.tracks[static_cast<size_t>(track)].cue;
}

} // namespace sequencer
