#include "ui/modules/pattern_editor.h"

#include <algorithm>

#include "songcore/effects.h"
#include "songcore/scales.h"
#include "ui/helpers.h"
#include "ui/matrix_geometry.h"

namespace pt::ui {

namespace {
using songcore::PhraseStep;


constexpr int BOTTOM_Y = 352;
constexpr Argb PATTERN_CURSOR_RED = 0xFFFF0000;
constexpr Argb INACTIVE_STEP_COLOR = 0xFF606060;

bool is_fx_parameter(PatternParameter p) {
    return p == PatternParameter::PAN || p == PatternParameter::SLIDE ||
           p == PatternParameter::CHANCE || p == PatternParameter::ARPEGGIATOR ||
           p == PatternParameter::FILTER_FREQUENCY || p == PatternParameter::RESONANCE ||
           p == PatternParameter::DRIVE || p == PatternParameter::CRUSH ||
           p == PatternParameter::DOWNSAMPLE || p == PatternParameter::REVERSE ||
           p == PatternParameter::REVERB || p == PatternParameter::DELAY;
}

int crush_parameter_value(const PhraseStep& step, PatternParameter p) {
    int slot = 0;
    for (int i = 1; i <= 3; ++i)
        if (songcore::step_fx_type(step, i) == songcore::FX_CRU) { slot = i; break; }
    if (!slot) return 0;
    const int value = songcore::step_fx_value(step, slot);
    return p == PatternParameter::CRUSH ? ((value >> 4) & 0x0F) : (value & 0x0F);
}

} // namespace

PatternParameter PatternEditorModule::footer_parameter(int index) {
    static constexpr PatternParameter params[] = {
        PatternParameter::NOTE, PatternParameter::INSTRUMENT, PatternParameter::VOLUME,
        PatternParameter::PAN, PatternParameter::SLIDE, PatternParameter::CHANCE,
        PatternParameter::ARPEGGIATOR, PatternParameter::MORE
    };
    const int i = std::clamp(index, 0, FOOTER_PARAMETER_COUNT - 1);
    return params[i];
}

const char* PatternEditorModule::parameter_label(PatternParameter p) {
    switch (p) {
        case PatternParameter::NOTE: return "N";
        case PatternParameter::INSTRUMENT: return "I";
        case PatternParameter::VOLUME: return "V";
        case PatternParameter::PAN: return "P";
        case PatternParameter::SLIDE: return "S";
        case PatternParameter::CHANCE: return "C";
        case PatternParameter::ARPEGGIATOR: return "A";
        case PatternParameter::FILTER_FREQUENCY: return "FQ";
        case PatternParameter::RESONANCE: return "RS";
        case PatternParameter::DRIVE: return "DV";
        case PatternParameter::CRUSH: return "CR";
        case PatternParameter::DOWNSAMPLE: return "DS";
        case PatternParameter::REVERSE: return "RV";
        case PatternParameter::REVERB: return "R";
        case PatternParameter::DELAY: return "D";
        case PatternParameter::CONDITION: return "C";
        case PatternParameter::WAIT: return "WAI";
        case PatternParameter::TRIGLESS: return "TRG";
        case PatternParameter::MORE: return "MORE";
    }
    return "";
}

const char* PatternEditorModule::parameter_name(PatternParameter p) {
    switch (p) {
        case PatternParameter::NOTE: return "Note";
        case PatternParameter::INSTRUMENT: return "Instrument";
        case PatternParameter::VOLUME: return "Volume";
        case PatternParameter::PAN: return "Pan";
        case PatternParameter::SLIDE: return "Slide";
        case PatternParameter::CHANCE: return "Chance";
        case PatternParameter::ARPEGGIATOR: return "Arpeggio";
        case PatternParameter::FILTER_FREQUENCY: return "Filter Frequency";
        case PatternParameter::RESONANCE: return "Resonance";
        case PatternParameter::DRIVE: return "Drive";
        case PatternParameter::CRUSH: return "Crush";
        case PatternParameter::DOWNSAMPLE: return "Downsample";
        case PatternParameter::REVERSE: return "Reverse";
        case PatternParameter::REVERB: return "Reverb";
        case PatternParameter::DELAY: return "Delay";
        case PatternParameter::CONDITION: return "Chance";
        case PatternParameter::WAIT: return "Wait";
        case PatternParameter::TRIGLESS: return "Trigless";
        case PatternParameter::MORE: return "All FX";
    }
    return "";
}

int PatternEditorModule::fx_code(PatternParameter p) {
    switch (p) {
        case PatternParameter::PAN: return songcore::FX_PAN;
        case PatternParameter::SLIDE: return songcore::FX_PSL;
        case PatternParameter::CHANCE: return songcore::FX_CHA;
        case PatternParameter::ARPEGGIATOR: return songcore::FX_ARPEGGIO;
        case PatternParameter::FILTER_FREQUENCY: return songcore::FX_CUT;
        case PatternParameter::RESONANCE: return songcore::FX_RES;
        case PatternParameter::DRIVE: return songcore::FX_DRV;
        case PatternParameter::CRUSH: return songcore::FX_CRU;
        case PatternParameter::DOWNSAMPLE: return songcore::FX_CRU;
        case PatternParameter::REVERSE: return songcore::FX_BCK;
        case PatternParameter::REVERB: return songcore::FX_RSEND;
        case PatternParameter::DELAY: return songcore::FX_DSEND;
        default: return songcore::FX_NONE;
    }
}

int PatternEditorModule::fx_slot(const PhraseStep& step, int code) {
    for (int slot = 1; slot <= 3; ++slot)
        if (songcore::step_fx_type(step, slot) == code) return slot;
    return 0;
}

int PatternEditorModule::ensure_fx_slot(PhraseStep& step, int code) {
    const int existing = fx_slot(step, code);
    if (existing) return existing;
    for (int slot = 1; slot <= 3; ++slot) {
        if (songcore::step_fx_type(step, slot) == songcore::FX_NONE) {
            songcore::step_set_fx(step, slot, code, 0);
            return slot;
        }
    }
    // SPRITESTEP has exactly three FX slots. If all are occupied, replace the last slot;
    // the UI's MORE picker is still available for authors who need to rearrange slots explicitly.
    songcore::step_set_fx(step, 3, code, 0);
    return 3;
}

bool PatternEditorModule::parameter_editable(PatternParameter p) {
    return p != PatternParameter::MORE;
}

bool PatternEditorModule::step_has_data(const PhraseStep& step) {
    return step.note != songcore::Note::EMPTY() || step.instrument != 0 || step.volume != 0x7F ||
           step.fx1Type != songcore::FX_NONE || step.fx2Type != songcore::FX_NONE ||
           step.fx3Type != songcore::FX_NONE;
}

bool PatternEditorModule::parameter_present(const PhraseStep& step, PatternParameter p) {
    if (p == PatternParameter::NOTE) return step.note != songcore::Note::EMPTY();
    if (p == PatternParameter::INSTRUMENT) return step.instrument != 0;
    if (p == PatternParameter::VOLUME) return step.volume != 0x7F;
    if (p == PatternParameter::CONDITION || p == PatternParameter::WAIT ||
        p == PatternParameter::TRIGLESS) return false;
    if (is_fx_parameter(p)) return fx_slot(step, fx_code(p)) != 0;
    return false;
}

int PatternEditorModule::parameter_value(const PhraseStep& step, PatternParameter p) {
    if (p == PatternParameter::INSTRUMENT) return step.instrument;
    if (p == PatternParameter::VOLUME) return step.volume;
    if (p == PatternParameter::NOTE)
        return step.note == songcore::Note::EMPTY() ? -1 : songcore::note_to_midi(step.note);
    if (p == PatternParameter::CRUSH || p == PatternParameter::DOWNSAMPLE)
        return crush_parameter_value(step, p);
    if (is_fx_parameter(p)) {
        const int slot = fx_slot(step, fx_code(p));
        return slot ? songcore::step_fx_value(step, slot) : 0;
    }
    return 0;
}

int PatternEditorModule::parameter_value(const sequencer::Pattern& pattern, int stepIndex, PatternParameter p) {
    if (stepIndex < 0 || stepIndex >= sequencer::MAX_PATTERN_STEPS) return 0;
    const size_t i = static_cast<size_t>(stepIndex);
    if (p == PatternParameter::CONDITION) return pattern.conditions[i];
    if (p == PatternParameter::WAIT) return pattern.wait_ppqn[i];
    if (p == PatternParameter::TRIGLESS) return pattern.trigless[i] ? 1 : 0;
    return parameter_value(pattern.steps[i], p);
}

void PatternEditorModule::set_parameter(PhraseStep& step, PatternParameter p, int value) {
    value = std::clamp(value, 0, 255);
    switch (p) {
        case PatternParameter::INSTRUMENT: step.instrument = value; return;
        case PatternParameter::VOLUME: step.volume = value; return;
        case PatternParameter::NOTE: step.note = songcore::note_from_midi(std::clamp(value, 0, 127)); return;
        case PatternParameter::CRUSH:
        case PatternParameter::DOWNSAMPLE: {
            const int slot = ensure_fx_slot(step, songcore::FX_CRU);
            int packed = songcore::step_fx_value(step, slot);
            if (p == PatternParameter::CRUSH) packed = (packed & 0x0F) | ((value & 0x0F) << 4);
            else packed = (packed & 0xF0) | (value & 0x0F);
            songcore::step_set_fx_value(step, slot, packed);
            return;
        }
        default: break;
    }
    if (is_fx_parameter(p)) {
        const int slot = ensure_fx_slot(step, fx_code(p));
        songcore::step_set_fx_value(step, slot, value);
    }
}

void PatternEditorModule::set_parameter(sequencer::Pattern& pattern, int stepIndex, PatternParameter p, int value) {
    if (stepIndex < 0 || stepIndex >= sequencer::MAX_PATTERN_STEPS) return;
    const size_t i = static_cast<size_t>(stepIndex);
    if (p == PatternParameter::CONDITION) {
        pattern.conditions[i] = static_cast<uint8_t>(std::clamp(value, 0, 0xFF));
        return;
    }
    if (p == PatternParameter::WAIT) {
        pattern.wait_ppqn[i] = static_cast<uint8_t>(std::clamp(value, 0, 0xFF));
        return;
    }
    if (p == PatternParameter::TRIGLESS) {
        pattern.trigless[i] = value != 0 ? 1 : 0;
        return;
    }
    set_parameter(pattern.steps[i], p, value);
}

void PatternEditorModule::clear_parameter(PhraseStep& step, PatternParameter p) {
    if (p == PatternParameter::NOTE) { step.note = songcore::Note::EMPTY(); return; }
    if (p == PatternParameter::INSTRUMENT) { step.instrument = 0; return; }
    if (p == PatternParameter::VOLUME) { step.volume = 0x7F; return; }
    if (p == PatternParameter::CRUSH || p == PatternParameter::DOWNSAMPLE) {
        const int slot = fx_slot(step, songcore::FX_CRU);
        if (!slot) return;
        int packed = songcore::step_fx_value(step, slot);
        if (p == PatternParameter::CRUSH) packed &= 0x0F;
        else packed &= 0xF0;
        songcore::step_set_fx_value(step, slot, packed);
        if (packed == 0) songcore::step_set_fx(step, slot, songcore::FX_NONE, 0);
        return;
    }
    if (is_fx_parameter(p)) {
        const int slot = fx_slot(step, fx_code(p));
        if (slot) songcore::step_set_fx(step, slot, songcore::FX_NONE, 0);
    }
}

void PatternEditorModule::clear_parameter(sequencer::Pattern& pattern, int stepIndex, PatternParameter p) {
    if (stepIndex < 0 || stepIndex >= sequencer::MAX_PATTERN_STEPS) return;
    const size_t i = static_cast<size_t>(stepIndex);
    if (p == PatternParameter::CONDITION) { pattern.conditions[i] = 0; return; }
    if (p == PatternParameter::WAIT) { pattern.wait_ppqn[i] = 0; return; }
    if (p == PatternParameter::TRIGLESS) { pattern.trigless[i] = 0; return; }
    clear_parameter(pattern.steps[i], p);
}

std::string PatternEditorModule::parameter_text(const PhraseStep& step, PatternParameter p) {
    if (p == PatternParameter::NOTE)
        return step.note == songcore::Note::EMPTY() ? "---" : note_name(step.note);
    if (p == PatternParameter::INSTRUMENT) return hex2(step.instrument);
    if (p == PatternParameter::VOLUME) return hex2(step.volume);
    if (p == PatternParameter::CRUSH || p == PatternParameter::DOWNSAMPLE)
        return hex1(crush_parameter_value(step, p));
    if (is_fx_parameter(p)) {
        const int slot = fx_slot(step, fx_code(p));
        return slot ? hex2(songcore::step_fx_value(step, slot)) : "--";
    }
    return "--";
}

namespace {
std::string pattern_parameter_text(const sequencer::Pattern& pattern, int stepIndex, PatternParameter p) {
    const size_t i = static_cast<size_t>(stepIndex);
    if (p == PatternParameter::CONDITION) {
        const int v = pattern.conditions[i];
        if (v == 0) return "--";
        const int n = (v >> 4) & 0x0F;
        const int d = v & 0x0F;
        return std::to_string(n) + "/" + std::to_string(d);
    }
    if (p == PatternParameter::WAIT) return pattern.wait_ppqn[i] ? hex2(pattern.wait_ppqn[i]) : "--";
    if (p == PatternParameter::TRIGLESS) return pattern.trigless[i] ? "ON" : "--";
    return PatternEditorModule::parameter_text(pattern.steps[i], p);
}
}

void PatternEditorModule::draw(Canvas& c, int x, int y, const PatternEditorState& s) const {
    const Theme& t = s.theme;
    c.fill_rect(x, y, WIDTH, HEIGHT, t.background);

    c.draw_text("PATTERN", x + 10, y + 5, t.textTitle, CHAR_SPACING, FONT_SCALE);
    c.draw_text("T" + std::to_string(s.track + 1) + "  B" + hex1(s.bank) + " P" + hex1(s.patternIndex),
                x + 130, y + 5, t.textParam, CHAR_SPACING, FONT_SCALE);

    const int iconY = y + 4;
    const int dirX = x + 438;
    const int shuffleX = x + 482;
    const Argb dirInk = s.headerControl == 1 ? PATTERN_CURSOR_RED : t.textParam;
    const Argb shuffleInk = s.headerControl == 2 ? PATTERN_CURSOR_RED : t.textParam;
    const int d = s.direction;
    if (d == 0) c.draw_text(">", dirX, iconY + 4, dirInk, CHAR_SPACING, FONT_SCALE);
    else if (d == 1) c.draw_text("<>", dirX - 5, iconY + 4, dirInk, CHAR_SPACING, FONT_SCALE);
    else if (d == 2) c.draw_text("<", dirX, iconY + 4, dirInk, CHAR_SPACING, FONT_SCALE);
    else c.draw_text("?", dirX, iconY + 4, dirInk, CHAR_SPACING, FONT_SCALE);
    c.fill_rect(shuffleX + 1, iconY + 4, 16, 2, shuffleInk);
    c.fill_rect(shuffleX + 5, iconY + 9, 16, 2, shuffleInk);
    c.fill_rect(shuffleX + 1, iconY + 14, 16, 2, shuffleInk);
    c.draw_text(hex2(s.shuffle), shuffleX + 25, iconY + 4, shuffleInk, CHAR_SPACING, FONT_SCALE);

    // Eight project tracks are rows. Each row displays the pattern selected for that track in Banks.
    // The step grid is deliberately fixed at 16 columns; the pattern's end marker is the small
    // triangle above its final active column.
    for (int track = 0; track < sequencer::TRACK_COUNT; ++track) {
        const int rowY = y + matrix::cell_y(track);
        const sequencer::Pattern* p = s.patterns[static_cast<size_t>(track)];
        const int pat  = s.patternIndices[static_cast<size_t>(track)];
        const bool selectedTrack = track == s.track;

        const int multiplier = std::max(1, s.stepDurationMultipliers[static_cast<size_t>(track)]);
        c.draw_text(std::to_string(multiplier), x + 2, rowY + 8,
                    (selectedTrack && s.rateLengthHighlight) ? PATTERN_CURSOR_RED :
                        (selectedTrack ? cursor_mark_ink(t) : t.textParam), CHAR_SPACING, FONT_SCALE);

        for (int step = 0; step < STEP_COUNT; ++step) {
            const int sx = x + matrix::cell_x(step);
            const bool inPattern = p && step < p->clamped_length();
            if (!inPattern) {
                // Steps beyond LEN remain visible as a darker, inactive grid.  They are still
                // useful as an addressable 16-step frame, but are not part of playback.
                const int mx = sx + matrix::empty_offset();
                const int my = rowY + matrix::empty_offset();
                c.fill_rect(mx, my, matrix::EMPTY_SIZE, matrix::EMPTY_SIZE, INACTIVE_STEP_COLOR);
                if (selectedTrack && step == s.cursorStep)
                    matrix::draw_cursor(c, sx, rowY, PATTERN_CURSOR_RED);
                continue;
            }

            const auto& data = p->steps[static_cast<size_t>(step)];
            const size_t si = static_cast<size_t>(step);
            const bool active = step_has_data(data) || p->conditions[si] != 0 ||
                                p->wait_ppqn[si] != 0 || p->trigless[si] != 0;
            const bool cursor = selectedTrack && step == s.cursorStep;
            if (active || cursor) {
                const Argb fill = cursor ? t.rowCursor : t.textValue;
                c.fill_rect(sx, rowY, matrix::OCCUPIED_SIZE, matrix::OCCUPIED_SIZE, fill);

                // Only the NOTE parameter uses the two-line pitch display.  When another
                // parameter is selected, the cell must show THAT parameter's value even when the
                // step also contains a note.  Otherwise every occupied cell looks like a note and
                // it becomes impossible to tell which parameter is being edited.
                if (s.parameter == PatternParameter::NOTE && data.note != songcore::Note::EMPTY()) {
                    const std::string pitch = songcore::NOTE_NAMES[data.note.pitch];
                    c.draw_text(pitch, sx, rowY + 1,
                                cursor ? cursor_cell_ink(t) : t.background, CHAR_SPACING, FONT_SCALE);
                    c.draw_text(std::to_string(data.note.octave), sx + 10, rowY + 18,
                                cursor ? cursor_cell_ink(t) : t.background, CHAR_SPACING, FONT_SCALE);
                } else {
                    const std::string value = pattern_parameter_text(*p, step, s.parameter);
                    c.draw_text(value, sx + 2, rowY + 9,
                                cursor ? cursor_cell_ink(t) : t.background, CHAR_SPACING, FONT_SCALE);
                }
            } else {
                const int mx = sx + matrix::empty_offset();
                const int my = rowY + matrix::empty_offset();
                c.fill_rect(mx, my, matrix::EMPTY_SIZE, matrix::EMPTY_SIZE, matrix::EMPTY_COLOR);
            }

            // Four-quadrant conditional trigger indicator. The current condition model stores
            // occurrence/total (e.g. 1/3 as 0x13), so the visual marks the configured occurrence
            // within the four available repeat positions without changing the playback semantics.
            const uint8_t condition = p->conditions[si];
            if (condition != 0) {
                const int occurrence = (condition >> 4) & 0x0F;
                const int count = condition & 0x0F;
                constexpr int q = 7;
                constexpr int gap = 1;
                constexpr int qi = 2;
                const int qx = sx + matrix::OCCUPIED_SIZE - (q * 2 + gap) - 1;
                const int qy = rowY + matrix::OCCUPIED_SIZE - (q * 2 + gap) - 1;
                for (int r = 0; r < 4; ++r) {
                    const int qcol = r & 1;
                    const int qrow = r >> 1;
                    const bool configured = (r + 1 == occurrence) && (r + 1 <= count) && (r < 4);
                    const int xx = qx + qcol * (q + gap);
                    const int yy = qy + qrow * (q + gap);
                    c.fill_rect(xx, yy, q, q, configured ? 0xFF000000 : 0xFFA0A0A0);
                    c.stroke_rect(xx, yy, q, q, 0xFF000000, qi);
                }
            }

            // The editing cursor is an outline outside the 35px cell. It never obscures the
            // note/parameter text and is therefore still obvious on an occupied step.
            if (cursor)
                matrix::draw_cursor(c, sx, rowY, PATTERN_CURSOR_RED);
            // FMS-style playback markers. Every step in the active pattern gets a small square,
            // not only steps containing notes: this makes the transport position visible even during
            // rests. The actual playhead square grows from 9px to 13px and becomes filled, so the
            // movement is unmistakable on the RG40XXH's 640x480 display.
            const auto& playhead = s.playheads[static_cast<size_t>(track)];
            const bool playingStep = s.isPlaying && playhead.phraseId == pat && playhead.step == step;
            if (playingStep) {
                constexpr int marker = 13;
                const int mx = sx + matrix::empty_offset() - (marker - matrix::EMPTY_SIZE) / 2;
                const int my = rowY + matrix::empty_offset() - (marker - matrix::EMPTY_SIZE) / 2;
                c.fill_rect(mx, my, marker, marker, t.textPlayhead);
                c.stroke_rect(mx - 1, my - 1, marker + 2, marker + 2, t.textValue, 1);
            }
        }

        if (p) {
            const int last = p->clamped_length() - 1;
            const int tx = x + matrix::cell_x(last) + matrix::OCCUPIED_SIZE + 2;
            const int ty = rowY + 13;
            // Small right-pointing END triangle. Moving LEN moves this marker with the final
            // active column; steps after it are rendered with INACTIVE_STEP_COLOR.
            const Argb triangleInk = s.rateLengthHighlight ? PATTERN_CURSOR_RED : t.textParam;
            c.fill_rect(tx, ty, 4, 3, triangleInk);
            c.fill_rect(tx + 4, ty - 2, 4, 7, triangleInk);
            c.fill_rect(tx + 8, ty - 4, 4, 11, triangleInk);
        }
    }

    static constexpr PatternParameter params[] = {
        PatternParameter::NOTE, PatternParameter::INSTRUMENT, PatternParameter::VOLUME,
        PatternParameter::PAN, PatternParameter::SLIDE, PatternParameter::CHANCE,
        PatternParameter::ARPEGGIATOR, PatternParameter::MORE
    };
    c.draw_text(parameter_name(s.parameter), x + 12, y + 337,
                t.textParam, CHAR_SPACING, FONT_SCALE);
    int px = x + 12;
    for (PatternParameter p : params) {
        const std::string label = p == PatternParameter::MORE ? "ALL^" : parameter_label(p);
        const int w = std::max(30, Canvas::text_width(label, CHAR_SPACING, FONT_SCALE) + 12);
        const bool selected = p == s.parameter;
        if (selected) c.fill_rect(px - 3, y + BOTTOM_Y, w, 24, t.rowCursor);
        c.draw_text(label, px, y + BOTTOM_Y + 5,
                    selected ? cursor_cell_ink(t) : t.textParam, CHAR_SPACING, FONT_SCALE);
        px += w + 3;
    }

}

CursorContext PatternEditorModule::cursor_context(const PatternEditorState& s) const {
    const auto* current = s.patterns[static_cast<size_t>(s.track)];
    if (!current) return cc::none();
    const auto& step = current->steps[static_cast<size_t>(s.cursorStep)];
    switch (s.parameter) {
        case PatternParameter::NOTE:
            return cc::note(parameter_value(step, s.parameter), !parameter_present(step, s.parameter), s.scaleMask, s.scaleKey);
        case PatternParameter::INSTRUMENT:
            return cc::instrument(step.instrument);
        case PatternParameter::VOLUME:
            return cc::volume(step.volume);
        case PatternParameter::CONDITION: {
            const int v = current->conditions[static_cast<size_t>(s.cursorStep)];
            return cc::effect_value(v, 0, 0xFF);
        }
        case PatternParameter::WAIT: {
            const int v = current->wait_ppqn[static_cast<size_t>(s.cursorStep)];
            return cc::effect_value(v, 0, 0xFF);
        }
        case PatternParameter::TRIGLESS: {
            const int v = current->trigless[static_cast<size_t>(s.cursorStep)] ? 1 : 0;
            return cc::effect_value(v, 0, 1);
        }
        case PatternParameter::MORE:
            return cc::none();
        default: {
            if (!is_fx_parameter(s.parameter)) return cc::none();
            const int code = fx_code(s.parameter);
            const int slot = fx_slot(step, code);
            if (!slot)
                return cc::effect_value(0, 1, songcore::effect_value_max(code));
            return cc::effect_value(songcore::step_fx_value(step, slot), slot,
                                    songcore::effect_value_max(code));
        }
    }
}

PatternEditResult PatternEditorModule::handle_input(sequencer::Pattern& pattern, PatternEditorState& state,
                                                     const InputAction& action) const {
    PatternEditResult result;
    state.cursorStep = std::clamp(state.cursorStep, 0, pattern.clamped_length() - 1);
    auto& step = pattern.steps[static_cast<size_t>(state.cursorStep)];

    switch (action.type) {
        case ActionType::SET_VALUE:
            if (parameter_editable(state.parameter)) {
                set_parameter(pattern, state.cursorStep, state.parameter, action.value);
                result.modified = true;
            }
            break;
        case ActionType::DELETE:
            if (parameter_editable(state.parameter)) {
                clear_parameter(pattern, state.cursorStep, state.parameter);
                result.modified = true;
            }
            break;
        case ActionType::INSERT_DEFAULT:
            if (state.parameter == PatternParameter::NOTE) {
                set_parameter(step, state.parameter, 60);
                result.modified = true;
            }
            break;
        default:
            break;
    }
    return result;
}

} // namespace pt::ui
