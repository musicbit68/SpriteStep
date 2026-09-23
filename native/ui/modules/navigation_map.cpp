#include "ui/modules/navigation_map.h"

#include "ui/helpers.h"
#include "ui/navigation.h"

namespace pt::ui {

namespace {
constexpr int EMPTY_CELL = -1;

int column_layout(int col, int row) {
    // Visible SPRITESTEP map. Five horizontal major views, with only the
    // context screens that actually belong to the current column shown above
    // them. Mixer/Effects are shared and are painted across all columns.
    static const int L[5][5] = {
        // row 0          row 1      row 2       row 3                       row 4
        {(int)ScreenType::PROJECT, EMPTY_CELL, (int)ScreenType::ARRANGE,    (int)ScreenType::MIXER,   (int)ScreenType::EFFECTS},
        {(int)ScreenType::PROJECT, EMPTY_CELL, (int)ScreenType::BANKS,      (int)ScreenType::MIXER,   (int)ScreenType::EFFECTS},
        {(int)ScreenType::SCALE, EMPTY_CELL, (int)ScreenType::PATTERN, (int)ScreenType::MIXER, (int)ScreenType::EFFECTS},
        {(int)ScreenType::INST_POOL, EMPTY_CELL, (int)ScreenType::INSTRUMENT, (int)ScreenType::MIXER, (int)ScreenType::EFFECTS},
        {EMPTY_CELL,      EMPTY_CELL, (int)ScreenType::MODS,       (int)ScreenType::MIXER,   (int)ScreenType::EFFECTS},
    };
    if (col < 0 || col > 4) col = 2;
    int v = L[col][row];
    // Named enum values above are safe because this table is compiled in the
    // same namespace as ScreenType.
    return v;
}

}  // namespace

void NavigationMapModule::draw(Canvas& c, int x, int y, const NavigationMapState& s) const {
    const Theme& t = s.theme;

    c.fill_rect(x, y, WIDTH, HEIGHT, t.background);

    // Which column are we in? A shared screen has none of its own, so it uses the one we came from.
    const int screenCol  = screen_column(s.currentScreen);
    const int currentCol = (screenCol == -1) ? s.sourceColumn : screenCol;

    // The visible map is driven by the same five-column contract as navigation.h.
    int grid[5][5];
    for (int row = 0; row < 5; ++row)
        for (int col = 0; col < 5; ++col) grid[row][col] = EMPTY_CELL;

    const int col = (currentCol < 0 || currentCol > 4) ? 2 : currentCol;
    for (int row = 0; row < 5; ++row) grid[row][col] = column_layout(col, row);
    for (int ccol = 0; ccol < 5; ++ccol) {
        grid[3][ccol] = static_cast<int>(ScreenType::MIXER);
        grid[4][ccol] = static_cast<int>(ScreenType::EFFECTS);
    }

    for (int row = 0; row < 5; ++row) {
        for (int gcol = 0; gcol < 5; ++gcol) {
            const int cell = grid[row][gcol];
            if (cell == EMPTY_CELL) continue;  // empty cells are just background

            const ScreenType screen = static_cast<ScreenType>(cell);
            // PROJECT is shared by the first two main columns. Paint it once, centered between
            // ARRANGE and BANKS, so the map reads as one project node above both screens.
            const int        cellX  = x + (screen == ScreenType::PROJECT ? CELL_WIDTH / 2 : gcol * CELL_WIDTH);
            const int        cellY  = y + (row * CELL_HEIGHT);

            const bool isCurrent = (screen == s.currentScreen);

            const std::string label  = screen_short_label(screen);
            const int         labelW = Canvas::text_width(label, CHAR_SPACING, FONT_SCALE);

            c.draw_text(label, cellX + (CELL_WIDTH - labelW) / 2, cellY + 3,
                        isCurrent ? cursor_mark_ink(t) : t.textValue, CHAR_SPACING, FONT_SCALE);
        }
    }
}

}  // namespace pt::ui
