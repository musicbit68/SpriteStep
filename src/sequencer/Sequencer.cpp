#include "sequencer/Sequencer.h"
namespace ss {
std::vector<NoteEvent> Sequencer::eventsAt(Tick tick) const {
    std::vector<NoteEvent> out;
    if (tick % PPQ != 0) return out;
    for (TrackIndex t = 0; t < TRACK_COUNT; ++t) {
        const auto& track = project_.tracks[t];
        const auto& pattern = track.active();
        TrackPlayer player; player.setPattern(&pattern);
        const auto idx = player.stepAtTransport(tick);
        const auto& step = pattern.step(idx);
        const auto note = step.get(Parameter::Note);
        const auto inst = step.get(Parameter::Instrument);
        if (!note.set) continue;
        NoteEvent e;
        e.tick = tick; e.track = t;
        e.note = note.value;
        e.instrument = inst.set ? inst.value : 0;
        e.overrides = step.values;
        out.push_back(e);
    }
    return out;
}
}
