#include "ui/modules/banks_view.h"

#include <algorithm>
#include <string>

#include "ui/helpers.h"
#include "ui/matrix_geometry.h"

namespace pt::ui {
namespace {

constexpr int BANK_Y = 344;



void draw_cell(Canvas& c, const Theme& t, int x, int y, const std::string& text,
               bool selected, bool playing, bool cueBlink, bool filled) {
    const bool highlighted = selected || playing || cueBlink;
    if (highlighted) {
        c.fill_rect(x, y, matrix::OCCUPIED_SIZE, matrix::OCCUPIED_SIZE, 0xFFA0A0A0);
        c.draw_text(text, x + 10, y + 10, 0xFF000000, CHAR_SPACING, FONT_SCALE);
        if (selected)
            c.stroke_rect(x - 1, y - 1, matrix::OCCUPIED_SIZE + 2, matrix::OCCUPIED_SIZE + 2,
                          0xFFFF0000, 1);
    } else if (filled) {
        c.draw_text(text, x + 10, y + 10, 0xFFFFFFFF, CHAR_SPACING, FONT_SCALE);
    } else {
        c.draw_text(text, x + 10, y + 10, t.textEmpty, CHAR_SPACING, FONT_SCALE);
    }
}

bool pattern_filled(const sequencer::Pattern& p) {
    if (p.length != sequencer::MAX_PATTERN_STEPS) return true;
    for (size_t i = 0; i < p.steps.size(); ++i) {
        const auto& step = p.steps[i];
        if (step.note != songcore::Note::EMPTY() || step.instrument != 0 || step.volume != 0x7F ||
            step.fx1Type != songcore::FX_NONE || step.fx2Type != songcore::FX_NONE ||
            step.fx3Type != songcore::FX_NONE || p.conditions[i] != 0 || p.wait_ppqn[i] != 0 ||
            p.trigless[i] != 0) return true;
    }
    return false;
}

bool bank_filled(const sequencer::Project& project, int bank) {
    for (int track = 0; track < sequencer::TRACK_COUNT; ++track)
        for (int pattern = 0; pattern < sequencer::PATTERN_COUNT; ++pattern)
            if (pattern_filled(sequencer::pattern_at(project, track, bank, pattern))) return true;
    return false;
}

} // namespace

void BanksViewModule::draw(Canvas& c, int x, int y, const BanksViewState& state,
                           const sequencer::Project& project) const {
    const Theme& t = state.theme;
    c.fill_rect(x, y, WIDTH, HEIGHT, t.background);
    c.draw_text("BANK", x + 12, y + 5, t.textTitle, CHAR_SPACING, FONT_SCALE);

    for (int track = 0; track < sequencer::TRACK_COUNT; ++track) {
        const int rowY = y + matrix::cell_y(track);
        c.draw_text("?", x + 3, rowY + 8,
                     (state.cursorTrack == track && state.cursorColumn == -1) ? 0xFFFF0000 : t.textParam,
                     CHAR_SPACING, FONT_SCALE);

        const auto& ph = state.playheads[static_cast<size_t>(track)];
        const bool playingHere = state.isPlaying && ph.chainId == state.bank && ph.chainRow >= 0;
        const auto& cue = state.cues[static_cast<size_t>(track)];
        for (int p = 0; p < sequencer::PATTERN_COUNT; ++p) {
            const int px = x + matrix::cell_x(p);
            const bool playing = playingHere && ph.chainRow == p;
            const bool cueHere = cue.pending && cue.bank == state.bank && cue.pattern == p;
            const bool cueBlink = cueHere && blink_on(state.blinkPhaseMs, false);
            const bool cursor = !state.bankSelector && state.cursorTrack == track && state.cursorColumn == p;
            const bool filled = pattern_filled(sequencer::pattern_at(project, track, state.bank, p));
            draw_cell(c, t, px, rowY, hex1(p), cursor, playing, cueBlink, filled);
        }
    }

    c.draw_text("BANK", x + 12, y + BANK_Y + 5, t.textTitle, CHAR_SPACING, FONT_SCALE);
    for (int b = 0; b < sequencer::BANK_COUNT; ++b) {
        const int bx = x + 122 + b * 31;
        const bool selected = state.bank == b && state.bankSelector;
        const bool filled = bank_filled(project, b);
        if (selected) {
            c.fill_rect(bx, y + BANK_Y, matrix::OCCUPIED_SIZE, matrix::OCCUPIED_SIZE, 0xFFFFFFFF);
            c.draw_text(hex1(b), bx + 10, y + BANK_Y + 10, 0xFF000000, CHAR_SPACING, FONT_SCALE);
            c.stroke_rect(bx - 1, y + BANK_Y - 1, matrix::OCCUPIED_SIZE + 2, matrix::OCCUPIED_SIZE + 2, 0xFFFF0000, 1);
        } else if (filled) {
            c.draw_text(hex1(b), bx + 10, y + BANK_Y + 10, 0xFFFFFFFF, CHAR_SPACING, FONT_SCALE);
        } else {
            c.draw_text(hex1(b), bx + 10, y + BANK_Y + 10, t.textEmpty, CHAR_SPACING, FONT_SCALE);
        }
    }
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
        // Cursor columns 0..15 are pattern slots. ? is the action column to the left.
        state.cursorColumn = std::clamp(state.cursorColumn + (dx > 0 ? 1 : -1), -1, 15);
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
        case -1:
            result.operation = BanksOperation::RANDOMIZE_TRACK_PATTERN;
            // A changing seed is supplied by the app in production; a stable seed here keeps the
            // module deterministic for callers/tests that don't maintain their own RNG.
            sequencer::BanksController(project).randomize_track_pattern(
                result.track, result.bank, result.pattern, 0xBADC0DEu + result.track);
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
        case -1:
            result.operation = BanksOperation::RANDOMIZE_ALL_SELECTED;
            for (int track = 0; track < sequencer::TRACK_COUNT; ++track)
                sequencer::BanksController(project).randomize_track_pattern(
                    track, state.bank, selected_pattern(state, track), 0xA11CEu + track + state.bank * 17);
            break;
        default:
            result.operation = BanksOperation::CUE_ALL_TRACKS_PATTERN_COLUMN;
            result.pattern = state.cursorColumn;
            break;
    }
    return result;
}

} // namespace pt::ui
