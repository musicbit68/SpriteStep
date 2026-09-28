#include "ui/navigation.h"
#include <cassert>
#include <iostream>

using namespace pt::ui;

int main() {
    NavState s{};

    s.currentScreen = ScreenType::INST_POOL;
    s.previousColumn = 3;
    assert(navigate_left(s).screen == ScreenType::SCALE);

    s.currentScreen = ScreenType::SCALE;
    s.previousColumn = 2;
    assert(navigate_left(s).screen == ScreenType::PROJECT);
    assert(navigate_right(s).screen == ScreenType::INST_POOL);

    std::cout << "navigation tests: PASS\n";
    return 0;
}
