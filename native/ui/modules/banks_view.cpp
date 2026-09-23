#include "ui/modules/banks_view.h"

#include <algorithm>
#include <string>

#include "ui/helpers.h"
#include "ui/matrix_geometry.h"

namespace pt::ui {
namespace {

constexpr int BANK_Y = 344;



void draw_cell(Canvas& c, const Theme& t, int x, int y, const std::string& text,
               bool selected, bool playing, bool cueBlink) {
    // BANK's 0..F slots are addressable cells, so the slot itself is the occupied matrix cell.
    // The action columns use the same geometry but remain text-only until selected.
    if (playing || cueBlink || selected) {
        c.fill_rect(x, y, matrix::OCCUPIED_SIZE, matrix::OCCUPIED_SIZE, t.textValue);
        c.draw_text(text, x + 10, y + 10, t.background, CHAR_SPACING, FONT_SCALE);
    } else {
        c.fill_rect(x, y, matrix::OCCUPIED_SIZE, matrix::OCCUPIED_SIZE, t.rowEvery4th);
        c.draw_text(text, x + 10, y + 10, t.textParam, CHAR_SPACING, FONT_SCALE);
    }
}


} // namespace

void BanksViewModule::draw(Canvas& c, int x, int y, const BanksViewState& state,
                           const sequencer::Project& project) const {
    const Theme& t = state.theme;
    c.fill_rect(x, y, WIDTH, HEIGHT, t.background);
    c.draw_text("BANK", x + 12, y + 5, t.textTitle, CHAR_SPACING, FONT_SCALE);

    // Eight track rows. Each row is ? / X / 0..F. The white cell is the pattern currently selected
    // for that track; this is also the pattern that Pattern View and playback use.
    for (int track = 0; track < sequencer::TRACK_COUNT; ++track) {
        const int rowY = y + matrix::cell_y(track);
        const int selected = selected_pattern(state, track);

        c.draw_text("?", x + matrix::cell_x(-2) + 14, rowY + 4, t.textParam, CHAR_SPACING, FONT_SCALE);
        c.draw_text("X", x + matrix::cell_x(-1) + 14, rowY + 4, t.textParam, CHAR_SPACING, FONT_SCALE);

        const auto& ph = state.playheads[static_cast<size_t>(track)];
        // chainId/chainRow are the handheld UI carrier for bank/pattern. The host now fills these
        // from runtime state even when the pattern has no sounding event, so the playing indicator
        // remains correct during rests and on empty patterns.
        const bool playingHere = state.isPlaying && ph.chainId == state.bank && ph.chainRow >= 0;
        const auto& cue = state.cues[static_cast<size_t>(track)];
        for (int p = 0; p < sequencer::PATTERN_COUNT; ++p) {
            const int px = x + matrix::cell_x(p);
            const bool playing = playingHere && ph.chainRow == p;
            const bool cueHere = cue.pending && cue.bank == state.bank && cue.pattern == p;
            const bool cueBlink = cueHere && blink_on(state.blinkPhaseMs, false);
            draw_cell(c, t, px, rowY, hex1(p), p == selected, playing, cueBlink);
        }

        // Keep the model reference live in this view: an empty pattern is visually distinct from a
        // populated one without changing the compact BANK layout.
        const auto& pattern = sequencer::pattern_at(project, track, state.bank, selected);
        if (pattern.length != sequencer::MAX_PATTERN_STEPS)
            c.draw_text(std::to_string(pattern.clamped_length()), x + 2, rowY + 5,
                        t.textParam, CHAR_SPACING, FONT_SCALE);
    }

    c.draw_text("BANK", x + 12, y + BANK_Y + 5, t.textTitle, CHAR_SPACING, FONT_SCALE);
    for (int b = 0; b < sequencer::BANK_COUNT; ++b) {
        const int bx = x + 122 + b * 31;
        const bool selected = state.bank == b && state.bankSelector;
        draw_cell(c, t, bx, y + BANK_Y, hex1(b), selected, false, false);
    }

    // Bottom-right MAP cue is kept compact so the navigation layer can replace it later.
    c.draw_text("Pabpim", x + 465, y + BANK_Y + 5, t.textParam, CHAR_SPACING, FONT_SCALE);
}

void BanksViewModule::move(BanksViewState& state, int dx, int dy) const {
    if (state.bankSelector) {
        if (dx != 0) state.bank = std::clamp(state.bank + (dx > 0 ? 1 : -1), 0, sequencer::BANK_COUNT - 1);
        if (dy < 0) state.bankSelector = false;
        return;
    }

    if (dy != 0) {
        state.cursorTrack = std::clamp(state.cursorTrack + (dy > 0 ? 1 : -1), 0,
                                       sequencer::TRACK_COUNT - 1);
    }

    if (dx != 0) {
        // Cursor columns 0..15 are pattern slots. ? and X are action columns to the left.
        state.cursorColumn = std::clamp(state.cursorColumn + (dx > 0 ? 1 : -1), -2, 15);
        if (state.cursorColumn >= 0)
            state.selectedPatterns[static_cast<size_t>(state.cursorTrack)] = state.cursorColumn;
    }

    if (dy > 0 && state.cursorTrack == sequencer::TRACK_COUNT - 1 && dx == 0)
        state.bankSelector = true;
}

BanksActionResult BanksViewModule::activate_b(BanksViewState& state, sequencer::Project& project) const {
    BanksActionResult result;
    result.track = state.cursorTrack;
    result.bank = state.bank;
    result.pattern = selected_pattern(state, state.cursorTrack);

    if (state.bankSelector) {
        state.bankSelector = false;
        return result;
    }

    switch (state.cursorColumn) {
        case -2:
            result.operation = BanksOperation::RANDOMIZE_TRACK_PATTERN;
            // A changing seed is supplied by the app in production; a stable seed here keeps the
            // module deterministic for callers/tests that don't maintain their own RNG.
            sequencer::BanksController(project).randomize_track_pattern(
                result.track, result.bank, result.pattern, 0xBADC0DEu + result.track);
            break;
        case -1:
            result.operation = BanksOperation::CLEAR_TRACK_PATTERN;
            sequencer::BanksController(project).clear_track_pattern(result.track, result.bank, result.pattern);
            break;
        default:
            result.operation = BanksOperation::CUE_TRACK_PATTERN;
            break;
    }
    return result;
}

BanksActionResult BanksViewModule::activate_a(BanksViewState& state, sequencer::Project& project) const {
    BanksActionResult result;
    result.track = state.cursorTrack;
    result.bank = state.bank;
    result.pattern = selected_pattern(state, state.cursorTrack);

    if (state.bankSelector) return result;

    switch (state.cursorColumn) {
        case -2:
            result.operation = BanksOperation::RANDOMIZE_ALL_SELECTED;
            for (int track = 0; track < sequencer::TRACK_COUNT; ++track)
                sequencer::BanksController(project).randomize_track_pattern(
                    track, state.bank, selected_pattern(state, track), 0xA11CEu + track + state.bank * 17);
            break;
        case -1:
            result.operation = BanksOperation::CLEAR_ALL_SELECTED;
            for (int track = 0; track < sequencer::TRACK_COUNT; ++track)
                sequencer::BanksController(project).clear_track_pattern(
                    track, state.bank, selected_pattern(state, track));
            break;
        default:
            result.operation = BanksOperation::CUE_ALL_TRACKS_PATTERN_COLUMN;
            result.pattern = state.cursorColumn;
            break;
    }
    return result;
}

} // namespace pt::ui
