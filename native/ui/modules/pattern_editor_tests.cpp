#include "ui/modules/pattern_editor.h"
#include "ui/fx_helper.h"

#include <cassert>
#include <iostream>
#include <vector>

using namespace pt::ui;
using namespace songcore;

int main() {
    sequencer::Pattern p;
    p.length = 8;
    constexpr unsigned C_MAJOR = (1u << 0) | (1u << 2) | (1u << 4) | (1u << 5) |
                                  (1u << 7) | (1u << 9) | (1u << 11);

    assert(!PatternEditorModule::step_has_data(p.steps[0]));
    assert(!PatternEditorModule::parameter_present(p.steps[0], PatternParameter::VOLUME));

    PatternEditorState state{{}, {}, {}, {}, 0, 0, 0, 0, PatternParameter::NOTE, {}, theme_classic()};
    state.patterns[0] = &p;
    state.cursorStep = 2;

    PatternEditorModule module;
    auto result = module.handle_input(p, state, InputAction::set_value(60));
    assert(result.modified);
    assert(p.steps[2].note == Note::C4());

    state.parameter = PatternParameter::PAN;
    result = module.handle_input(p, state, InputAction::set_value(0x55));
    assert(result.modified);
    assert(step_has_fx(p.steps[2], FX_PAN));
    assert(PatternEditorModule::parameter_value(p.steps[2], PatternParameter::PAN) == 0x55);

    state.parameter = PatternParameter::REVERB;
    result = module.handle_input(p, state, InputAction::set_value(0x33));
    assert(result.modified);
    assert(step_has_fx(p.steps[2], FX_RSEND));
    assert(PatternEditorModule::parameter_value(p.steps[2], PatternParameter::REVERB) == 0x33);

    result = module.handle_input(p, state, InputAction::of(ActionType::DELETE));
    assert(result.modified);
    assert(!step_has_fx(p.steps[2], FX_RSEND));

    state.parameter = PatternParameter::CONDITION;
    result = module.handle_input(p, state, InputAction::set_value(0x13));
    assert(result.modified);
    assert(p.conditions[2] == 0x13);
    assert(PatternEditorModule::parameter_value(p, 2, PatternParameter::CONDITION) == 0x13);
    result = module.handle_input(p, state, InputAction::of(ActionType::DELETE));
    assert(result.modified);
    assert(p.conditions[2] == 0);


    state.parameter = PatternParameter::WAIT;
    result = module.handle_input(p, state, InputAction::set_value(3));
    assert(result.modified);
    assert(p.wait_ppqn[2] == 3);
    assert(PatternEditorModule::parameter_value(p, 2, PatternParameter::WAIT) == 3);
    result = module.handle_input(p, state, InputAction::of(ActionType::DELETE));
    assert(result.modified);
    assert(p.wait_ppqn[2] == 0);

    state.parameter = PatternParameter::TRIGLESS;
    result = module.handle_input(p, state, InputAction::set_value(1));
    assert(result.modified);
    assert(p.trigless[2] == 1);
    assert(PatternEditorModule::parameter_value(p, 2, PatternParameter::TRIGLESS) == 1);
    auto trigCtx = module.cursor_context(state);
    assert(trigCtx.valueType == CursorValueType::EFFECT_VALUE);
    assert(trigCtx.currentValue == 1);
    assert(trigCtx.maxValue == 1);
    result = module.handle_input(p, state, InputAction::of(ActionType::DELETE));
    assert(result.modified);
    assert(p.trigless[2] == 0);


    state.parameter = PatternParameter::CHANCE;
    result = module.handle_input(p, state, InputAction::set_value(0x55));
    assert(result.modified);
    assert(step_has_fx(p.steps[2], FX_CHA));
    assert(PatternEditorModule::parameter_value(p.steps[2], PatternParameter::CHANCE) == 0x55);

    state.parameter = PatternParameter::CRUSH;
    result = module.handle_input(p, state, InputAction::set_value(0x0A));
    assert(result.modified);
    state.parameter = PatternParameter::DOWNSAMPLE;
    result = module.handle_input(p, state, InputAction::set_value(0x05));
    assert(result.modified);
    int cruSlot = 0;
    for (int slot = 1; slot <= 3; ++slot) if (step_fx_type(p.steps[2], slot) == FX_CRU) cruSlot = slot;
    assert(cruSlot != 0);
    assert(step_fx_value(p.steps[2], cruSlot) == 0xA5);
    assert(PatternEditorModule::parameter_value(p.steps[2], PatternParameter::CRUSH) == 0x0A);
    assert(PatternEditorModule::parameter_value(p.steps[2], PatternParameter::DOWNSAMPLE) == 0x05);

    state.parameter = PatternParameter::REVERSE;
    result = module.handle_input(p, state, InputAction::set_value(0x7F));
    assert(result.modified);
    assert(step_has_fx(p.steps[2], FX_BCK));

    state.parameter = PatternParameter::VOLUME;
    result = module.handle_input(p, state, InputAction::set_value(0x40));
    assert(result.modified);
    assert(p.steps[2].volume == 0x40);

    const auto ctx = module.cursor_context(state);
    assert(ctx.valueType == CursorValueType::VOLUME);
    assert(ctx.currentValue == 0x40);

    // FMS-style Range Edit: the selected parameter is edited independently on every step in the
    // horizontal span, using each step's own CursorContext.
    state.parameter = PatternParameter::VOLUME;
    p.steps[1].volume = 0x20;
    p.steps[2].volume = 0x40;
    p.steps[3].volume = 0x60;
    state.cursorStep = 1;
    const bool rangeChanged = module.apply_range_action(p, state, 1, 3, pt::ui::increment);
    assert(rangeChanged);
    assert(p.steps[1].volume == 0x21);
    assert(p.steps[2].volume == 0x41);
    assert(p.steps[3].volume == 0x61);

    // Range note editing remains scale-aware on each individual step.
    state.parameter = PatternParameter::NOTE;
    state.scaleMask = C_MAJOR;
    state.scaleKey = 0;
    p.steps[1].note = Note::C4();
    p.steps[2].note = songcore::note_from_midi(62);
    p.steps[3].note = songcore::note_from_midi(64);
    const bool noteRangeChanged = module.apply_range_action(p, state, 1, 3, pt::ui::increment);
    assert(noteRangeChanged);
    assert(songcore::note_to_midi(p.steps[1].note) == 62);
    assert(songcore::note_to_midi(p.steps[2].note) == 64);
    assert(songcore::note_to_midi(p.steps[3].note) == 65);

    // ALL^ FX parameter editing: the picker selects an effect code, then the normal A+DPAD
    // editing path can create/adjust that effect without reopening the picker.
    state.parameter = PatternParameter::MORE;
    state.selectedFxCode = FX_PAN;
    state.cursorStep = 4;
    auto fxCtx = module.cursor_context(state);
    assert(fxCtx.valueType == CursorValueType::EFFECT_VALUE);
    result = module.handle_input(p, state, InputAction::set_value(0x44));
    assert(result.modified);
    assert(step_has_fx(p.steps[4], FX_PAN));
    assert(PatternEditorModule::parameter_value(p.steps[4], PatternParameter::PAN) == 0x44);
    result = module.handle_input(p, state, InputAction::of(ActionType::DELETE));
    assert(result.modified);
    assert(!step_has_fx(p.steps[4], FX_PAN));

    // Parameter randomization: one gesture changes only the selected parameter, and an empty cell
    // receives a real note so the randomized value is immediately meaningful during playback.
    state.parameter = PatternParameter::VOLUME;
    state.selectedFxCode = FX_NONE;
    state.cursorStep = 5;
    const std::vector<int> instrumentPool{2, 7, 11};
    assert(PatternEditorModule::randomize_parameter(p, 5, PatternParameter::VOLUME, FX_NONE,
                                                    0x12345678u, C_MAJOR, 0, &instrumentPool));
    assert(p.steps[5].note != Note::EMPTY());
    assert(p.steps[5].volume >= 48 && p.steps[5].volume <= 127);

    assert(PatternEditorModule::randomize_parameter(p, 6, PatternParameter::PAN, FX_NONE,
                                                    0xCAFEBABEu, C_MAJOR, 0, &instrumentPool));
    assert(step_has_fx(p.steps[6], FX_PAN));

    assert(PatternEditorModule::randomize_parameter(p, 7, PatternParameter::MORE, FX_DRV,
                                                    0xDEADBEEFu, C_MAJOR, 0, &instrumentPool));
    assert(step_has_fx(p.steps[7], FX_DRV));
    int drvSlot = 0;
    for (int slot = 1; slot <= 3; ++slot) if (step_fx_type(p.steps[7], slot) == FX_DRV) drvSlot = slot;
    assert(drvSlot != 0);
    assert(step_fx_value(p.steps[7], drvSlot) <= 192);

    // ALL^ deliberately omits destructive/playback/meta commands and parameters already exposed in
    // the Pattern footer, while retaining useful per-step sound-shaping effects.
    const auto& allFx = fx_layout_pattern_all();
    assert(allFx.count() == 1);
    const auto& codes = allFx.groups[0].codes;
    auto hasFx = [&](int code) {
        for (int c : codes) if (c == code) return true;
        return false;
    };
    assert(!hasFx(FX_KILL)); assert(!hasFx(FX_HOP)); assert(!hasFx(FX_GRV));
    assert(!hasFx(FX_RND)); assert(!hasFx(FX_RNL)); assert(!hasFx(FX_TBL));
    assert(!hasFx(FX_THO)); assert(!hasFx(FX_TIC)); assert(!hasFx(FX_AUS)); assert(!hasFx(FX_AUF));
    assert(!hasFx(FX_VOLUME)); assert(!hasFx(FX_PAN)); assert(!hasFx(FX_PSL));
    assert(!hasFx(FX_CHA)); assert(!hasFx(FX_ARPEGGIO));
    assert(hasFx(FX_RSEND)); assert(hasFx(FX_DSEND));
    assert(hasFx(FX_DRV)); assert(hasFx(FX_CRU)); assert(hasFx(FX_CUT)); assert(hasFx(FX_RES));

    std::cout << "pattern editor tests: PASS\n";
    return 0;
}
