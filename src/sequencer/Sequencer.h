#pragma once
#include "project/Project.h"
#include "sequencer/TrackPlayer.h"
#include "sequencer/PatternLauncher.h"
#include "sequencer/Transport.h"
#include "events/NoteEvent.h"
#include <vector>

namespace ss {
class Sequencer {
public:
    explicit Sequencer(Project& project) : project_(project), transport_(project.bpm) {}
    Transport& transport() { return transport_; }
    const Transport& transport() const { return transport_; }
    std::vector<NoteEvent> eventsAt(Tick tick) const;
    PatternLauncher& launcher(TrackIndex t) { return launchers_.at(t); }
private:
    Project& project_;
    Transport transport_;
    std::array<PatternLauncher, TRACK_COUNT> launchers_{};
};
}
