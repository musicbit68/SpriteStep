#include "banks_view.h"

#include <cassert>
#include <iostream>

int main() {
    sequencer::Project project;
    pt::ui::BanksViewModule view;
    pt::ui::BanksViewState state;

    // Pattern cursor movement is track-local.
    state.cursorTrack = 2;
    state.cursorColumn = 0;
    view.move(state, 1, 0);
    assert(state.cursorColumn == 1);
    assert(state.selectedPatterns[2] == 1);

    // The X clear column is gone; ? + B randomizes only the selected track/pattern.
    state.cursorColumn = -1;
    auto result = view.activate_b(state, project);
    assert(result.operation == pt::ui::BanksOperation::RANDOMIZE_TRACK_PATTERN);

    // The randomizer receives one instrument per track and applies it consistently to every
    // generated note in that track's pattern.
    state.cursorColumn = -1;
    state.randomInstruments.fill(5);
    view.activate_b(state, project);
    for (const auto& step : project.tracks[2].banks[0].patterns[1].steps)
        if (step.note != songcore::Note::EMPTY()) assert(step.instrument == 5);

    // When playback is active, ? + B targets the pattern actually sounding on the selected track,
    // even if the UI cursor is still on a different bank/slot.
    state.isPlaying = true;
    state.playingBanks[2] = 3;
    state.playingPatterns[2] = 9;
    state.bank = 0;
    state.selectedPatterns[2] = 1;
    state.cursorColumn = -1;
    state.randomSeed = 0x12345678u;
    result = view.activate_b(state, project);
    assert(result.operation == pt::ui::BanksOperation::RANDOMIZE_TRACK_PATTERN);
    assert(result.bank == 3 && result.pattern == 9);
    assert(!sequencer::pattern_at(project, 2, 3, 9).steps.empty());

    // A second randomize seed must produce a fresh pattern in the same currently-playing slot.
    const auto firstRandomized = project.tracks[2].banks[3].patterns[9];
    state.randomSeed = 0x87654321u;
    view.activate_b(state, project);
    const auto secondRandomized = project.tracks[2].banks[3].patterns[9];
    bool different = false;
    for (size_t i = 0; i < firstRandomized.steps.size(); ++i) {
        if (firstRandomized.steps[i].note != secondRandomized.steps[i].note ||
            firstRandomized.steps[i].volume != secondRandomized.steps[i].volume ||
            firstRandomized.steps[i].fx1Type != secondRandomized.steps[i].fx1Type ||
            firstRandomized.steps[i].fx1Value != secondRandomized.steps[i].fx1Value) {
            different = true;
            break;
        }
    }
    assert(different);
    // The ALL randomizer also targets each track's actual playing bank/slot and uses distinct
    // streams, rather than copying one generated pattern into every track.
    state.isPlaying = true;
    state.randomSeed = 0x10203040u;
    for (int track = 0; track < sequencer::TRACK_COUNT; ++track) {
        state.playingBanks[static_cast<size_t>(track)] = 4;
        state.playingPatterns[static_cast<size_t>(track)] = 5;
        state.selectedPatterns[static_cast<size_t>(track)] = 0;
    }
    state.cursorColumn = -1;
    result = view.activate_a(state, project);
    assert(result.operation == pt::ui::BanksOperation::RANDOMIZE_ALL_SELECTED);
    for (int track = 0; track < sequencer::TRACK_COUNT; ++track) {
        const auto& pattern = project.tracks[static_cast<size_t>(track)].banks[4].patterns[5];
        bool hasNoteOrFx = false;
        for (const auto& step : pattern.steps) {
            if (step.note != songcore::Note::EMPTY() || step.fx1Type != songcore::FX_NONE ||
                step.fx2Type != songcore::FX_NONE || step.fx3Type != songcore::FX_NONE) {
                hasNoteOrFx = true;
                break;
            }
        }
        assert(hasNoteOrFx);
    }
    state.isPlaying = false;

    // Pattern + B is a cue request, not an immediate pattern replacement.
    state.cursorColumn = 7;
    state.selectedPatterns.fill(7);
    result = view.activate_b(state, project);
    assert(result.operation == pt::ui::BanksOperation::CUE_TRACK_PATTERN);
    assert(result.track == 2 && result.bank == 0 && result.pattern == 7);

    // ? + A randomizes the selected pattern slot on every track.
    state.cursorColumn = -1;
    result = view.activate_a(state, project);
    assert(result.operation == pt::ui::BanksOperation::RANDOMIZE_ALL_SELECTED);

    // A + pattern is a cue-column request for all tracks.
    state.cursorColumn = 4;
    result = view.activate_a(state, project);
    assert(result.operation == pt::ui::BanksOperation::CUE_ALL_TRACKS_PATTERN_COLUMN);
    assert(result.bank == 0 && result.pattern == 4);

    // The D-pad never enters the footer bank numbers. Bank changes are a modifier gesture.
    state.cursorTrack = sequencer::TRACK_COUNT - 1;
    state.cursorColumn = 4;
    state.bankSelector = false;
    view.move(state, 0, 1);
    assert(!state.bankSelector);
    assert(state.cursorTrack == sequencer::TRACK_COUNT - 1);

    std::cout << "banks view tests: PASS\n";
}
