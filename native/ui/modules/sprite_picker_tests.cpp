#include "ui/modules/sprite_picker.h"
#include <cassert>
#include <iostream>

using namespace pt::ui;

int main() {
    SpritePickerState s;
    s.open = true;
    assert(sprite_picker_sprite_id(s) == -1);
    sprite_picker_move(s, 1);
    assert(sprite_picker_sprite_id(s) == 0);
    sprite_picker_move(s, 127);
    assert(sprite_picker_sprite_id(s) == 127);
    sprite_picker_move(s, 1);
    assert(sprite_picker_sprite_id(s) == -1); // wraps through NO SPRITE
    sprite_picker_random(s, 12345);
    assert(sprite_picker_sprite_id(s) >= 0 && sprite_picker_sprite_id(s) < 128);
    assert(SPRITE_COUNT == 128);
    std::cout << "sprite picker tests: PASS\n";
}
