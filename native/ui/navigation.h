#pragma once

// SPRITESTEP navigation map.
//
// The old PocketTracker/Songcore editor hierarchy (Song -> Chain -> Phrase -> Table)
// is retained internally for compatibility with the legacy document model, but it is
// NOT part of the handheld UI. The visible five-screen horizontal row is:
//
//   ARRANGE  BANKS  PATTERN  INSTRUMENT  MODS
//
// Vertical context screens are deliberately kept in their original modules:
//
//   PATTERN      SCALE
//   INSTRUMENT   INST.POOL
//   all columns  MIXER -> EFFECTS
//
// R+LEFT/RIGHT moves along the five main SPRITESTEP screens. R+UP/DOWN moves within
// the current column. This is the navigation contract used by the MAP renderer too.

#include "app_state.h"
#include "cursor_move.h"
#include "screen.h"

#include <algorithm>

namespace pt::ui {

struct NavResult {
    ScreenType screen = ScreenType::PATTERN;
    int column = 2;
    bool instrumentFromPool = false;
};

struct NavState {
    ScreenType currentScreen = ScreenType::PATTERN;
    int previousColumn = 2;
    bool instrumentFromPool = false;
};

inline int screen_column(ScreenType s) {
    switch (s) {
        case ScreenType::ARRANGE:    return 0;
        case ScreenType::BANKS:      return 1;
        case ScreenType::PATTERN:    return 2;
        case ScreenType::INSTRUMENT: return 3;
        case ScreenType::MODS:       return 4;
        case ScreenType::SCALE:      return 2;
        case ScreenType::INST_POOL:  return 3;
        default: return -1; // shared/context/legacy/popup screens
    }
}

inline ScreenType main_screen_for_column(int column) {
    switch (column) {
        case 0: return ScreenType::ARRANGE;
        case 1: return ScreenType::BANKS;
        case 2: return ScreenType::PATTERN;
        case 3: return ScreenType::INSTRUMENT;
        case 4: return ScreenType::MODS;
        default: return ScreenType::PATTERN;
    }
}

inline bool is_main_row(ScreenType s) {
    for (ScreenType m : MAIN_ROW_SCREENS)
        if (m == s) return true;
    return false;
}

namespace detail {
inline int context_column(const NavState& s) {
    const int c = screen_column(s.currentScreen);
    return c == -1 ? s.previousColumn : c;
}

inline bool is_vertical_context(ScreenType s) {
    return s == ScreenType::SCALE || s == ScreenType::INST_POOL ||
           s == ScreenType::MIXER || s == ScreenType::EFFECTS;
}

inline bool is_spritestep_screen(ScreenType s) {
    return is_main_row(s) || is_vertical_context(s);
}
}

inline NavResult navigate_up(const NavState& s) {
    const int col = detail::context_column(s);
    switch (s.currentScreen) {
        case ScreenType::EFFECTS:    return {ScreenType::MIXER, col};
        case ScreenType::MIXER:      return {main_screen_for_column(col), col};
        case ScreenType::SCALE:      return {ScreenType::SCALE, 2};
        case ScreenType::INST_POOL:  return {ScreenType::INST_POOL, 3};
        case ScreenType::PATTERN:    return {ScreenType::SCALE, 2};
        case ScreenType::INSTRUMENT: return {ScreenType::INST_POOL, 3};
        case ScreenType::MODS:       return {ScreenType::INSTRUMENT, 3};
        case ScreenType::ARRANGE:
        case ScreenType::BANKS:      return {s.currentScreen, col};
        default:                     return {s.currentScreen, col};
    }
}

inline NavResult navigate_down(const NavState& s) {
    const int col = detail::context_column(s);
    switch (s.currentScreen) {
        case ScreenType::SCALE:      return {ScreenType::PATTERN, 2};
        case ScreenType::INST_POOL:  return {ScreenType::INSTRUMENT, 3};
        case ScreenType::MIXER:     return {ScreenType::EFFECTS, col};
        case ScreenType::EFFECTS:   return {ScreenType::EFFECTS, col};
        case ScreenType::ARRANGE:
        case ScreenType::BANKS:
        case ScreenType::PATTERN:
        case ScreenType::INSTRUMENT:
        case ScreenType::MODS:      return {ScreenType::MIXER, col};
        default:                    return {s.currentScreen, col};
    }
}

inline NavResult navigate_left(const NavState& s) {
    const int col = detail::context_column(s);
    if (is_main_row(s.currentScreen)) {
        return {main_screen_for_column(std::max(0, col - 1)), std::max(0, col - 1)};
    }
    // From a vertical context, return to its owning main screen rather than
    // exposing any of the legacy Song/Chain/Phrase/Table screens.
    if (detail::is_spritestep_screen(s.currentScreen))
        return {main_screen_for_column(col), col};
    return {s.currentScreen, col};
}

inline NavResult navigate_right(const NavState& s) {
    const int col = detail::context_column(s);
    if (is_main_row(s.currentScreen)) {
        return {main_screen_for_column(std::min(4, col + 1)), std::min(4, col + 1)};
    }
    if (detail::is_spritestep_screen(s.currentScreen))
        return {main_screen_for_column(col), col};
    return {s.currentScreen, col};
}

// ─── Applying it ─────────────────────────────────────────────────────────────────

/** The NavState the four functions above want, read off the live AppState. */
inline NavState nav_state_of(const AppState& s) {
    return NavState{s.currentScreen, s.previousColumn, s.instrumentFromPool};
}

/**
 * Land on a screen. Everything Kotlin's `TrackerController.currentScreen` SETTER does, plus the
 * cursor save/restore its callers do around it — and none of it is bookkeeping you can skip:
 *
 *   • THE CURSOR MUST BE SAVED AND RESTORED. SONG, CHAIN and PHRASE share one `cursorColumn` but have
 *     8, 2 and 9 columns. Leave PHRASE on column 9, arrive on CHAIN, and the cursor is outside every
 *     cell — it does not clamp, it DISAPPEARS (no cell matches, so nothing draws highlighted). The
 *     per-screen slots are what make the shared cursor safe.
 *   • TABLE FOLLOWS THE INSTRUMENT. Arriving on TABLE syncs `currentTable` to `currentInstrument`,
 *     because a table is an instrument's automation and showing table 3 while instrument 7 is selected
 *     would be showing you someone else's.
 *   • THE POOL FLAG IS STICKY ONLY ON INSTRUMENT. Any move to a screen that is not INSTRUMENT clears
 *     it, so a stale flag cannot silently reroute a later R+LEFT back to the pool.
 */
inline void go_to_screen(AppState& s, const NavResult& r) {
    // Save where we were leaving from (REMEMBER mode reads these back).
    switch (s.currentScreen) {
        case ScreenType::SONG:
            s.songCursorRow = s.cursorRow;   s.songCursorColumn = s.cursorColumn;   break;
        case ScreenType::CHAIN:
            s.chainCursorRow = s.cursorRow;  s.chainCursorColumn = s.cursorColumn;  break;
        case ScreenType::PHRASE:
            s.phraseCursorRow = s.cursorRow; s.phraseCursorColumn = s.cursorColumn; break;
        default: break;
    }

    s.currentScreen  = r.screen;
    s.previousColumn = r.column;

    // The five SPRITESTEP major views own the horizontal R+LEFT/RIGHT ring.
    // Context screens (SCALE, INST.POOL, MIXER, EFFECTS) temporarily leave that ring.
    s.seqMajorViewContext = is_main_row(r.screen);
    s.instrumentFromPool = false;

    if (r.screen == ScreenType::TABLE) {
        // Kotlin runs this line through the currentTable SETTER (TrackerController.kt:124–129),
        // which clamps to the pool and mirrors lastEditedTable. Plain fields here, so both are
        // said out loud — assign the field bare and lastEditedTable trails one navigation behind.
        const int last    = static_cast<int>(s.project->tables.size()) - 1;
        s.currentTable    = std::min(last, std::max(0, s.currentInstrument));
        s.lastEditedTable = s.currentTable;
    }

    // Restore — or refresh, which is the Android default.
    //
    // ⚠️ UNDER NAV = SONG, SONG AND CHAIN ALWAYS RESTORE, whatever the CURSOR row says. Their cursors
    // are not a convenience there — they ARE the pointer (ui/song_pointer.h), so a REFRESH to row 0
    // would not lose your place, it would silently re-aim the whole thing at song row 0 / track 1.
    // PHRASE is untouched: its row is a step, not a pointer component, so REFRESH still means what it
    // has always meant.
    const bool pointerScreen = s.settings.navSongRelative &&
                               (r.screen == ScreenType::SONG || r.screen == ScreenType::CHAIN);
    if (s.settings.cursorRemember || pointerScreen) {
        switch (r.screen) {
            case ScreenType::SONG:
                s.cursorRow = s.songCursorRow;   s.cursorColumn = s.songCursorColumn;   break;
            case ScreenType::CHAIN:
                s.cursorRow = s.chainCursorRow;  s.cursorColumn = s.chainCursorColumn;  break;
            case ScreenType::PHRASE:
                s.cursorRow = s.phraseCursorRow; s.cursorColumn = s.phraseCursorColumn; break;
            // TABLE / GROOVE / the rest own their cursors outright — they persist by construction.
            default: break;
        }
    } else {
        // REFRESH — every screen that owns a cursor resets it to its top-left editable cell on entry
        // (bar the two the pointer owns under NAV = SONG, which took the branch above).
        //
        // ⚠️ INSTRUMENT, MODS and INST.POOL were MISSING here until S5 (their cursors persisted across
        // an entry, where Android's refresh them). Harmless-looking, but INSTRUMENT is the one screen
        // whose ROW MAP changes shape under it — a SoundFont has a different row list from a sampler —
        // so a stale row survives onto a map that may not have one, and the cursor silently draws
        // nowhere. `instrument_row_kind` is bounds-safe (out of range reads as SINGLE, as Kotlin's
        // getOrElse does), so it was never a crash; it was a cursor you could lose.
        //
        // EFFECTS is deliberately absent: Kotlin does not reset it either, so its row persists in BOTH
        // modes.
        switch (r.screen) {
            case ScreenType::SONG:
            case ScreenType::CHAIN:
            case ScreenType::PHRASE:
                s.cursorRow    = 0;
                s.cursorColumn = min_cursor_column(r.screen);  // 1 — never the read-only gutter
                break;
            case ScreenType::TABLE:
                s.tableCursorRow = 0; s.tableCursorColumn = 1;
                break;
            case ScreenType::GROOVE:
                s.grooveCursorRow = 0;
                break;
            case ScreenType::INSTRUMENT:
                s.instrumentCursorRow = 0; s.instrumentCursorColumn = 1;
                break;
            case ScreenType::MODS:
                s.modCursorRow = 0; s.modCursorPair = 0; s.modCursorSide = 0;
                break;
            case ScreenType::INST_POOL:
                s.poolCursorColumn = 0;   // …but NOT currentInstrument: that IS the pool's row
                break;
            case ScreenType::MIXER:
                s.mixerCursorColumn = 0; s.mixerMasterRow = 0;
                break;
            default: break;
        }
    }

    // ⚠️ A GUARD KOTLIN CANNOT NEED — and note that PROJECT and SETTINGS are absent from BOTH arms
    // above, deliberately: Kotlin never resets their cursors either, so they persist in both modes
    // (as EFFECTS' does). This is not a refresh. It is a BOUNDS check, and it exists only because the
    // SETTINGS row map is caps-FILTERED here and is not on Android.
    //
    // The cursor's default row is 0 = LAYOUT, a row the SHELL does not draw. Without this, the very
    // first entry into SETTINGS would leave the cursor on an invisible row: nothing highlighted
    // anywhere, and A+DPAD quietly editing a touch-layout setting on a device that has no touch
    // screen. Android's row 0 is always present, so its `restoreCursorForScreen` has nothing to clamp.
    if (r.screen == ScreenType::SETTINGS &&
        !settings_row_visible(static_cast<SettingsRow>(s.settingsCursorRow), s.caps)) {
        s.settingsCursorRow    = settings_first_visible_row(s.caps);
        s.settingsCursorColumn = 1;
    }

    // ⚠️ THE SAME BOUNDS CHECK FOR THE MIXER, and for the same reason: its two ints describe a grid
    // that is not rectangular, so a pair carried in under REMEMBER can name a cell the screen does not
    // draw — no cursor anywhere, and A+DPAD editing nothing. Row 0 is the one row that exists in every
    // column, so a lost cursor comes back on the fader above where it was rather than at the far left.
    if (r.screen == ScreenType::MIXER && !mixer_cell_exists(s.mixerMasterRow, s.mixerCursorColumn)) {
        s.mixerMasterRow    = 0;
        s.mixerCursorColumn = (s.mixerCursorColumn >= 0 && s.mixerCursorColumn <= 8)
                                  ? s.mixerCursorColumn
                                  : 0;
    }

    // SONG's viewport must contain its cursor, whichever branch above set it.
    if (r.screen == ScreenType::SONG) scroll_song_to_row(s, s.cursorRow);

    // ⭐ And under NAV = SONG the two current* refs are a READING of the cursors just saved and
    // restored above — so this is where they are taken, once, below every site that lands a screen
    // change, rather than at each of the thirty that consume them. A no-op under NAV = POOL.
    refresh_song_relative_refs(s);
}

}  // namespace pt::ui
