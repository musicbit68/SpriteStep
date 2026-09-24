#ifndef SPRITESTEP_SEQUENCER_SPRITESTEP_ADAPTER_H
#define SPRITESTEP_SEQUENCER_SPRITESTEP_ADAPTER_H

#include <cstdint>
#include <cstddef>
#include <array>

#include "sequencer.h"
#include "songcore/model.h"
#include "songcore/router.h"
#include "songcore/scheduler.h"

namespace sequencer {

class SPRITESTEPAdapter {
public:
    SPRITESTEPAdapter(Project& sequenceProject, songcore::Project& audioProject,
                         songcore::MidiRouter& router, int sampleRate = 44100);

    void start(int scene = 0, int64_t frame = 0);
    void start_banks(int bank, const std::array<int, TRACK_COUNT>& patterns, int64_t frame = 0);
    void stop();
    bool playing() const { return sequencer_.playing(); }
    size_t schedule_until(int64_t endFrame);

    Sequencer& sequencer() { return sequencer_; }
    const Sequencer& sequencer() const { return sequencer_; }
    songcore::Sequencer& songcore_scheduler() { return scheduler_; }
    const songcore::Sequencer& songcore_scheduler() const { return scheduler_; }

private:
    Project& sequenceProject_;
    songcore::Project& audioProject_;
    Sequencer sequencer_;
    songcore::Sequencer scheduler_;
};

} // namespace sequencer

#endif
