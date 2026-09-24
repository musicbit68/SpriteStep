#pragma once

#include <array>
#include <algorithm>
#include <cstdint>

#include "sequencer/banks.h"
#include "ui/playhead.h"
#include "ui/canvas.h"
#include "ui/cursor.h"
#include "ui/theme.h"

namespace pt::ui {

// Cursor columns: -1 = randomize, 0..15 = pattern slot.
struct BanksViewState {
    int bank = 0;
    std::array<int, sequencer::TRACK_COUNT> selectedPatterns{};
    std::array<sequencer::TrackCue, sequencer::TRACK_COUNT> cues{};
    std::array<TrackPlayhead, sequencer::TRACK_COUNT> playheads{};
    int blinkPhaseMs = 0;
    bool isPlaying = false;
    int cursorTrack = 0;
    int cursorColumn = 0;
    bool bankSelector = false;
    Theme theme = theme_classic();

    BanksViewState() { selectedPatterns.fill(0); }
};

enum class BanksOperation {
    NONE,
    CUE_TRACK_PATTERN,
    CUE_ALL_TRACKS_PATTERN_COLUMN,
    RANDOMIZE_TRACK_PATTERN,
    CLEAR_TRACK_PATTERN,
    RANDOMIZE_ALL_SELECTED,
    CLEAR_ALL_SELECTED
};

struct BanksActionResult {
    BanksOperation operation = BanksOperation::NONE;
    int track = -1;
    int bank = 0;
    int pattern = 0;
};

class BanksViewModule {
public:
    static constexpr int WIDTH = 610;
    static constexpr int HEIGHT = 390;

    void draw(Canvas& c, int x, int y, const BanksViewState& state,
              const sequencer::Project& project) const;

    // D-pad navigation. Pattern selection is intentionally per-track; changing the bank changes
    // which 16 slots are shown, while each row remembers its selected slot.
    void move(BanksViewState& state, int dx, int dy) const;

    // BANKS action columns: ? is randomize. The X clear column was removed from the handheld UI.
    BanksActionResult activate_b(BanksViewState& state, sequencer::Project& project) const;
    BanksActionResult activate_a(BanksViewState& state, sequencer::Project& project) const;

private:
    static int column_for_pattern(int pattern) { return std::clamp(pattern, 0, 15); }
    static int selected_pattern(const BanksViewState& state, int track) {
        return state.selectedPatterns[static_cast<size_t>(track)];
    }
};

} // namespace pt::ui
