#include "ui/modules/pattern_editor.h"

#include <algorithm>

#include "songcore/effects.h"
#include "songcore/scales.h"
#include "ui/helpers.h"

namespace pt::ui {

namespace {
using songcore::PhraseStep;

constexpr int GRID_X = 22;
constexpr int GRID_Y = 42;
constexpr int CELL = 35;
constexpr int ROW_H = 37;
constexpr int BOTTOM_Y = 352;

bool is_fx_parameter(PatternParameter p) {
    return p == PatternParameter::PAN || p == PatternParameter::SLIDE ||
           p == PatternParameter::CONDITION || p == PatternParameter::ARPEGGIATOR ||
           p == PatternParameter::M1 || p == PatternParameter::M2 || p == PatternParameter::M3 ||
           p == PatternParameter::M4 || p == PatternParameter::REVERB || p == PatternParameter::DELAY;
}

} // namespace

const char* PatternEditorModule::parameter_label(PatternParameter p) {
    switch (p) {
        case PatternParameter::INSTRUMENT: return "I";
        case PatternParameter::NOTE: return "NOTE";
        case PatternParameter::VOLUME: return "VOL";
        case PatternParameter::PAN: return "PAN";
        case PatternParameter::SLIDE: return "SLI";
        case PatternParameter::CONDITION: return "CHA";
        case PatternParameter::ARPEGGIATOR: return "ARP";
        case PatternParameter::M1: return "M1";
        case PatternParameter::M2: return "M2";
        case PatternParameter::M3: return "M3";
        case PatternParameter::M4: return "M4";
        case PatternParameter::REVERB: return "REV";
        case PatternParameter::DELAY: return "DEL";
        case PatternParameter::WAIT: return "WAI";
        case PatternParameter::TRIGLESS: return "TRG";
        case PatternParameter::MORE: return "MORE";
    }
    return "";
}

int PatternEditorModule::fx_code(PatternParameter p) {
    switch (p) {
        case PatternParameter::PAN: return songcore::FX_PAN;
        case PatternParameter::SLIDE: return songcore::FX_PSL;
        case PatternParameter::CONDITION: return songcore::FX_NONE;
        case PatternParameter::ARPEGGIATOR: return songcore::FX_ARPEGGIO;
        case PatternParameter::M1: return songcore::FX_CCA;
        case PatternParameter::M2: return songcore::FX_CCB;
        case PatternParameter::M3: return songcore::FX_CCC;
        case PatternParameter::M4: return songcore::FX_CCD;
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

    // Eight project tracks are rows. Each row displays the pattern selected for that track in Banks.
    // The step grid is deliberately fixed at 16 columns; the pattern's end marker is the small
    // triangle above its final active column.
    for (int track = 0; track < sequencer::TRACK_COUNT; ++track) {
        const int rowY = y + GRID_Y + track * ROW_H;
        const sequencer::Pattern* p = s.patterns[static_cast<size_t>(track)];
        const int pat  = s.patternIndices[static_cast<size_t>(track)];
        const bool selectedTrack = track == s.track;

        const int multiplier = std::max(1, s.stepDurationMultipliers[static_cast<size_t>(track)]);
        c.draw_text(std::to_string(multiplier), x + 2, rowY + 8,
                    selectedTrack ? cursor_mark_ink(t) : t.textParam, CHAR_SPACING, FONT_SCALE);

        for (int step = 0; step < STEP_COUNT; ++step) {
            const int sx = x + GRID_X + step * CELL;
            const bool inPattern = p && step < p->clamped_length();
            if (!inPattern) {
                c.fill_rect(sx, rowY, CELL - 2, 34, darken(t.background, 1.35f));
                continue;
            }

            const auto& data = p->steps[static_cast<size_t>(step)];
            const size_t si = static_cast<size_t>(step);
            const bool active = step_has_data(data) || p->conditions[si] != 0 ||
                                p->wait_ppqn[si] != 0 || p->trigless[si] != 0;
            const bool cursor = selectedTrack && step == s.cursorStep;
            const Argb fill = cursor ? t.rowCursor : active ? t.textValue : t.rowEvery4th;
            c.fill_rect(sx, rowY, CELL - 3, 34, fill);
            if (active || cursor) {
                const std::string value = pattern_parameter_text(*p, step, s.parameter);
                c.draw_text(value, sx + 2, rowY + 9,
                            cursor ? cursor_cell_ink(t) : t.background, CHAR_SPACING, FONT_SCALE);
            }
            // FMS-style playback marker: every active step gets a small square, and the
            // currently sounding step grows slightly.  Use the per-track playhead array so
            // Pattern View can show all eight independent tracks at once.
            const auto& playhead = s.playheads[static_cast<size_t>(track)];
            const bool playingStep = playhead.phraseId == pat && playhead.step == step;
            if (active) {
                const int marker = playingStep ? 13 : 9;
                const int mx = sx + (CELL - 3 - marker) / 2;
                const int my = rowY + 34 - marker - 3;
                c.stroke_rect(mx, my, marker, marker, t.textPlayhead, playingStep ? 2 : 1);
            }
        }

        if (p) {
            const int last = p->clamped_length() - 1;
            const int tx = x + GRID_X + last * CELL + 13;
            const int ty = rowY - 5;
            for (int k = 0; k < 3; ++k)
                c.fill_rect(tx - k * 4, ty + k * 2, 8 + k * 2, 2, t.textParam);
        }
    }

    static constexpr PatternParameter params[] = {
        PatternParameter::INSTRUMENT, PatternParameter::NOTE, PatternParameter::VOLUME,
        PatternParameter::PAN, PatternParameter::SLIDE, PatternParameter::CONDITION,
        PatternParameter::ARPEGGIATOR, PatternParameter::M1, PatternParameter::M2,
        PatternParameter::M3, PatternParameter::M4, PatternParameter::REVERB,
        PatternParameter::DELAY, PatternParameter::WAIT, PatternParameter::TRIGLESS,
        PatternParameter::MORE
    };
    int px = x + 12;
    for (PatternParameter p : params) {
        const std::string label = parameter_label(p);
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
            return cc::note(parameter_value(step, s.parameter), !parameter_present(step, s.parameter));
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
