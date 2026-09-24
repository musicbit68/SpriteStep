#include "spritestep_adapter.h"

namespace sequencer {

SPRITESTEPAdapter::SPRITESTEPAdapter(Project& sequenceProject, songcore::Project& audioProject,
                                           songcore::MidiRouter& router, int sampleRate)
    : sequenceProject_(sequenceProject),
      audioProject_(audioProject),
      sequencer_(sequenceProject, sampleRate),
      scheduler_(router, audioProject, sampleRate) {
    scheduler_.set_clock(0);
}

void SPRITESTEPAdapter::start(int scene, int64_t frame) {
    sequencer_.start_arrange(scene, frame);
    scheduler_.set_clock(frame);
}

void SPRITESTEPAdapter::start_banks(int bank, const std::array<int, TRACK_COUNT>& patterns, int64_t frame) {
    sequencer_.start_banks(bank, patterns, frame);
    scheduler_.set_clock(frame);
}

void SPRITESTEPAdapter::stop() {
    sequencer_.stop();
}

size_t SPRITESTEPAdapter::schedule_until(int64_t endFrame) {
    const auto events = sequencer_.schedule_until(endFrame);
    for (const ScheduledStep& event : events) {
        if (event.track < 0 || event.track >= TRACK_COUNT) continue;
        if (event.bank >= BANK_COUNT || event.pattern >= PATTERN_COUNT) continue;

        const Pattern& pattern = pattern_at(sequenceProject_, event.track, event.bank, event.pattern);
        if (event.step < 0 || event.step >= pattern.clamped_length()) continue;

        PatternStep step = pattern.steps[static_cast<size_t>(event.step)];
        // SPRITESTEP already treats an EMPTY-note PhraseStep as a parameter/FX-only step.
        // Convert the handheld trigless metadata into that native representation so the scheduler
        // evaluates the step's FX without emitting a new note trigger.
        if (event.trigless) step.note = songcore::Note::EMPTY();

        scheduler_.schedule_step_for_track(step, event.frame, event.duration_frames, event.track);
    }
    scheduler_.set_clock(endFrame);
    return events.size();
}

} // namespace sequencer
