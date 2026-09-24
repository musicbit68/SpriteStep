#include "ui/modules/pattern_editor.h"

#include <cassert>
#include <iostream>

using namespace pt::ui;
using namespace songcore;

int main() {
    sequencer::Pattern p;
    p.length = 8;

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

    std::cout << "pattern editor tests: PASS\n";
    return 0;
}
