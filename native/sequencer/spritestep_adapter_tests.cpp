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
    audioProject.tempo = 60;

    Capture capture;
    songcore::MidiRouter router(&capture);
    sequencer::SPRITESTEPAdapter adapter(sequenceProject, audioProject, router, 44100);
    adapter.start();

    // The adapter's sequence model does not own project tempo; the full songcore project does.
    // Starting at 60 BPM must therefore produce a sixteenth-note grid of 11025 frames at 44.1kHz.
    const int64_t stepFrames = adapter.sequencer().base_step_frames();
    assert(stepFrames == 11025);
    const size_t count = adapter.schedule_until(stepFrames + 1);
    assert(count == 1);
    assert(capture.note_count == 1);
    assert(capture.events.front().type == songcore::EV_NOTE_ON);
    assert(capture.events.front().frame == 0);
    assert(capture.events.front().track == 0);

    // Changing the project tempo while the adapter is already running must be observed on the next
    // scheduling pass. The current step boundary remains intact; subsequent steps use the new tempo.
    const int64_t oldStepFrames = stepFrames;
    adapter.stop();
    adapter.songcore_scheduler().reset_track_state(0);
    std::array<int, sequencer::TRACK_COUNT> selected{};
    selected.fill(0);
    audioProject.tempo = 60;
    adapter.start_banks(0, selected);
    adapter.schedule_until(oldStepFrames + 1);
    audioProject.tempo = 120;
    const size_t tempoCount = adapter.schedule_until(oldStepFrames * 2 + 1);
    assert(tempoCount >= 2);
    assert(adapter.sequencer().base_step_frames() == 5512);

    capture.events.clear();
    capture.note_count = 0;
    adapter.stop();
    adapter.songcore_scheduler().reset_track_state(0);
    audioProject.tempo = 60;
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
