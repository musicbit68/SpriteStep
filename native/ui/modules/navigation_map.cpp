#include "ui/modules/navigation_map.h"

#include "ui/helpers.h"
#include "ui/navigation.h"

namespace pt::ui {

namespace {
constexpr Argb MAP_EMPTY = 0xFF303030;
constexpr Argb MAP_CURRENT = 0xFFA0A0A0;
constexpr int CELL = 9;
constexpr int GAP = 3;
constexpr int MAIN_W = 5 * CELL + 4 * GAP;
constexpr int BAR_H = 7;
constexpr int BAR_Y1 = 35;
constexpr int BAR_Y2 = 45;
constexpr int TOP_Y = 0;
constexpr int MAIN_Y = 13;

void map_cell(Canvas& c, int x, int y, bool current) {
    c.fill_rect(x, y, CELL, CELL, current ? MAP_CURRENT : MAP_EMPTY);
}

} // namespace

void NavigationMapModule::draw(Canvas& c, int x, int y, const NavigationMapState& s) const {
    // Compact block map matching the SPRITESTEP mockup. The middle row is the five main pages;
    // the three blocks above it are PROJECT / SCALE / INST.POOL, and the two bars are MIXER / EFFECTS.
    // There is deliberately no text: a single grey block is the current page, all others remain dark.
    c.fill_rect(x, y, WIDTH, HEIGHT, s.theme.background);

    const int screenCol = screen_column(s.currentScreen);
    const int currentCol = (screenCol == -1) ? s.sourceColumn : screenCol;

    // Context pages above their owning main page.
    map_cell(c, x + CELL + GAP / 2, y + TOP_Y, s.currentScreen == ScreenType::PROJECT);
    map_cell(c, x + 2 * (CELL + GAP), y + TOP_Y, s.currentScreen == ScreenType::SCALE);
    map_cell(c, x + 3 * (CELL + GAP), y + TOP_Y, s.currentScreen == ScreenType::INST_POOL);

    for (int col = 0; col < 5; ++col)
        map_cell(c, x + col * (CELL + GAP), y + MAIN_Y,
                 s.currentScreen == main_screen_for_column(col));

    c.fill_rect(x, y + BAR_Y1, MAIN_W, BAR_H,
                s.currentScreen == ScreenType::MIXER ? MAP_CURRENT : MAP_EMPTY);
    c.fill_rect(x, y + BAR_Y2, MAIN_W, BAR_H,
                s.currentScreen == ScreenType::EFFECTS ? MAP_CURRENT : MAP_EMPTY);
}

}  // namespace pt::ui
