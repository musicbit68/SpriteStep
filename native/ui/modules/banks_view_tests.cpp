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

    // Moving onto X then B clears only the selected track/pattern.
    state.cursorColumn = -1;
    auto& selected = sequencer::pattern_at(project, 2, 0, 1);
    selected.steps[0].note = songcore::Note::C4();
    selected.length = 5;
    auto result = view.activate_b(state, project);
    assert(result.operation == pt::ui::BanksOperation::CLEAR_TRACK_PATTERN);
    assert(selected.length == 5);
    assert(selected.steps[0].note == songcore::Note::EMPTY());

    // ? + B randomizes only that track.
    state.cursorColumn = -2;
    result = view.activate_b(state, project);
    assert(result.operation == pt::ui::BanksOperation::RANDOMIZE_TRACK_PATTERN);

    // Pattern + B is a cue request, not an immediate pattern replacement.
    state.cursorColumn = 7;
    state.selectedPatterns.fill(7);
    state.selectedPatterns[2] = 7;
    result = view.activate_b(state, project);
    assert(result.operation == pt::ui::BanksOperation::CUE_TRACK_PATTERN);
    assert(result.track == 2 && result.bank == 0 && result.pattern == 7);

    // A + X clears the selected slot on every track, preserving lengths.
    for (int t = 0; t < sequencer::TRACK_COUNT; ++t) {
        auto& p = sequencer::pattern_at(project, t, 0, 7);
        p.length = static_cast<uint8_t>(t + 1);
        p.steps[0].note = songcore::Note::C4();
    }
    state.cursorColumn = -1;
    result = view.activate_a(state, project);
    assert(result.operation == pt::ui::BanksOperation::CLEAR_ALL_SELECTED);
    for (int t = 0; t < sequencer::TRACK_COUNT; ++t) {
        const auto& p = sequencer::pattern_at(project, t, 0, 7);
        assert(p.length == t + 1);
        assert(p.steps[0].note == songcore::Note::EMPTY());
    }

    // A + pattern is a cue-column request for all tracks.
    state.cursorColumn = 4;
    result = view.activate_a(state, project);
    assert(result.operation == pt::ui::BanksOperation::CUE_ALL_TRACKS_PATTERN_COLUMN);
    assert(result.pattern == 4);

    std::cout << "banks view tests: PASS\n";
}
