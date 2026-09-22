#include "spritestep_adapter.h"

#include <cassert>
#include <iostream>

namespace {
struct Capture : songcore::IMidiConsumer {
    int note_count = 0;
    std::vector<songcore::Event> events;
    void consume(const songcore::Event& ev) override { events.push_back(ev); if (ev.type == songcore::EV_NOTE_ON) ++note_count; }
    void on_play(const std::string&, const std::string&, int64_t, int, int) override {}
    void on_stop() override {}
};
}

int main() {
    sequencer::Project sequenceProject;
    sequenceProject.tracks[0].banks[0].patterns[0].length = 1;
    sequenceProject.scenes.push_back({});
    sequenceProject.scenes[0].tracks[0].active = true;
    sequenceProject.scenes[0].tracks[0].bank = 0;
    sequenceProject.scenes[0].tracks[0].pattern = 0;
    auto& step = sequenceProject.tracks[0].banks[0].patterns[0].steps[0];
    step.note = songcore::Note::C4();
    step.instrument = 0;
    step.volume = 0x7F;

    songcore::Project audioProject = songcore::make_default_project();
    audioProject.tempo = 120;

    Capture capture;
    songcore::MidiRouter router(&capture);
    sequencer::SPRITESTEPAdapter adapter(sequenceProject, audioProject, router, 44100);
    adapter.start();

    const int64_t stepFrames = adapter.sequencer().base_step_frames();
    const size_t count = adapter.schedule_until(stepFrames + 1);
    assert(count == 1);
    assert(capture.note_count == 1);
    assert(capture.events.front().type == songcore::EV_NOTE_ON);
    assert(capture.events.front().frame == 0);
    assert(capture.events.front().track == 0);

    capture.events.clear();
    capture.note_count = 0;
    adapter.stop();
    adapter.songcore_scheduler().reset_track_state(0);
    sequenceProject.tracks[0].banks[0].patterns[0].steps[0].note = songcore::Note::EMPTY();
    sequenceProject.tracks[0].banks[0].patterns[0].steps[0].fx1Type = songcore::FX_VOLUME;
    sequenceProject.tracks[0].banks[0].patterns[0].steps[0].fx1Value = 0x40;
    adapter.start();
    adapter.schedule_until(stepFrames + 1);

    bool sawVolume = false;
    for (const auto& ev : capture.events) {
        if (ev.type == songcore::EV_CC && ev.cc.param == songcore::CC_VOLUME) {
            sawVolume = true;
            break;
        }
    }
    assert(sawVolume);

    // Trigless: retain the step payload for FX processing, but suppress the note trigger.
    adapter.stop();
    adapter.songcore_scheduler().reset_track_state(0);
    auto& triglessStep = sequenceProject.tracks[0].banks[0].patterns[0].steps[0];
    triglessStep.note = songcore::Note::C4();
    triglessStep.fx1Type = songcore::FX_VOLUME;
    triglessStep.fx1Value = 0x30;
    sequenceProject.tracks[0].banks[0].patterns[0].trigless[0] = 1;
    capture.events.clear();
    capture.note_count = 0;
    adapter.start();
    adapter.schedule_until(stepFrames + 1);
    assert(capture.note_count == 0);
    bool sawTriglessVolume = false;
    for (const auto& ev : capture.events) {
        if (ev.type == songcore::EV_CC && ev.cc.param == songcore::CC_VOLUME) {
            sawTriglessVolume = true;
            break;
        }
    }
    assert(sawTriglessVolume);

    std::cout << "SPRITESTEP adapter tests: PASS\n";
    return 0;
}
