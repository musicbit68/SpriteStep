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
