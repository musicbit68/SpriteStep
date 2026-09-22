#include "ui/modules/arrange_view.h"

#include <cassert>
#include <iostream>

using pt::ui::ActionType;
using pt::ui::ArrangeViewModule;
using pt::ui::InputAction;
using songcore::SequencerData;

static void test_macro_encoding() {
    SequencerData d;
    ArrangeViewModule v;
    auto r = v.handle_input(d, 0, 1, 2, InputAction::set_value(0x11));
    assert(r.modified && r.hasPattern);
    assert(d.scenes.size() == 3);
    assert(d.scenes[2].tracks[1].active);
    assert(d.scenes[2].tracks[1].bank == 1);
    assert(d.scenes[2].tracks[1].pattern == 1);
}

static void test_pages_are_scene_addressing() {
    SequencerData d;
    ArrangeViewModule v;
    auto r = v.handle_input(d, 1, 0, 0, InputAction::set_value(0x7F));
    assert(r.modified);
    assert(d.scenes.size() == 17);
    assert(d.scenes[16].tracks[0].bank == 7);
    assert(d.scenes[16].tracks[0].pattern == 15);
}

static void test_delete_returns_to_empty() {
    SequencerData d;
    ArrangeViewModule v;
    v.handle_input(d, 0, 0, 0, InputAction::set_value(0x00));
    assert(d.scenes.size() == 1);
    auto r = v.handle_input(d, 0, 0, 0, InputAction::of(ActionType::DELETE));
    assert(r.modified);
    assert(d.scenes.empty());
}

static void test_independent_tracks() {
    SequencerData d;
    ArrangeViewModule v;
    v.handle_input(d, 0, 0, 0, InputAction::set_value(0x00));
    v.handle_input(d, 0, 1, 0, InputAction::set_value(0x11));
    assert(d.scenes[0].tracks[0].active);
    assert(d.scenes[0].tracks[0].bank == 0);
    assert(d.scenes[0].tracks[0].pattern == 0);
    assert(d.scenes[0].tracks[1].bank == 1);
    assert(d.scenes[0].tracks[1].pattern == 1);
}

int main() {
    test_macro_encoding();
    test_pages_are_scene_addressing();
    test_delete_returns_to_empty();
    test_independent_tracks();
    std::cout << "arrange view tests: PASS\n";
    return 0;
}
