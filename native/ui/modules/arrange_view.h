#pragma once

// ─── ARRANGE VIEW ────────────────────────────────────────────────────────────────────────────────
//
// The handheld sequencer's linear arrangement.  Each of the eight rows is one track and each
// column is one scene.  A cell is a complete two-digit pattern macro: high nibble = bank, low
// nibble = pattern.  The cell is therefore only a reference; the pattern data remains in the
// track-local Banks library.
//
// The screen shows 16 scenes at a time.  Eight pages (0..7) provide 128 addressable scenes.  An
// untouched scene is implicit and costs no serialized data; editing a cell grows Project::scenes
// only as far as necessary.

#include "songcore/model.h"
#include "ui/canvas.h"
#include "ui/cursor.h"
#include "ui/theme.h"

namespace pt::ui {

struct ArrangeViewState {
    const songcore::SequencerData& sequencer;
    int page = 0;       // 0..7, sixteen scene columns per page
    int cursorRow = 0;  // 0..7 track
    int cursorColumn = 0; // 0..15 scene within page
    int playingScene = -1; // absolute scene index; -1 means stopped/not visible
    Theme theme = theme_classic();
};

struct ArrangeInputResult {
    bool modified = false;
    bool hasPattern = false;
    int lastEditedBank = 0;
    int lastEditedPattern = 0;
};

class ArrangeViewModule {
public:
    static constexpr int WIDTH = 610;
    static constexpr int HEIGHT = 392;
    static constexpr int TRACK_COUNT = songcore::SEQUENCER_TRACKS;
    static constexpr int SCENE_PAGE_COUNT = 8;
    static constexpr int SCENES_PER_PAGE = songcore::SEQUENCER_PATTERNS;
    static constexpr int MAX_SCENES = SCENE_PAGE_COUNT * SCENES_PER_PAGE;

    void draw(Canvas& c, int x, int y, const ArrangeViewState& s) const;

    CursorContext cursor_context(const ArrangeViewState& s) const;

    ArrangeInputResult handle_input(songcore::SequencerData& data, int page, int cursor_row,
                                    int cursor_column, const InputAction& action) const;

    static int absolute_scene(int page, int column);

private:
    static bool valid_page(int page) { return page >= 0 && page < SCENE_PAGE_COUNT; }
    static bool valid_column(int column) { return column >= 0 && column < SCENES_PER_PAGE; }
    static bool valid_track(int track) { return track >= 0 && track < TRACK_COUNT; }
    static bool valid_macro(int value) { return value >= 0 && value < 256; }

    static void ensure_scene(songcore::SequencerData& data, int scene);
    static int macro_value(const songcore::SequencerPatternRef& ref);
    static songcore::SequencerPatternRef ref_from_macro(int value);
    static bool scene_is_empty(const songcore::SequencerArrangeScene& scene);
};

} // namespace pt::ui
