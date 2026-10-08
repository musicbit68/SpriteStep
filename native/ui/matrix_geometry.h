#pragma once

// Shared geometry for the three SPRITESTEP sequencer matrices.  Arrange, Banks and Pattern
// deliberately use the same cell centers so the handheld UI has one visual grid rather than three
// subtly different ones.
namespace pt::ui::matrix {
inline constexpr int COLUMN_SPACING = 37;
inline constexpr int ROW_SPACING = 37;
inline constexpr int OCCUPIED_SIZE = 35;
inline constexpr int EMPTY_SIZE = 9;
inline constexpr unsigned EMPTY_COLOR = 0xFFA0A0A0;
inline constexpr int GRID_X = 18;
inline constexpr int GRID_Y = 42;

// Handheld matrix cursor: a 41px frame around each 35px cell, with 3px corner flags.
inline constexpr int CURSOR_SIZE = 41;
inline constexpr int CURSOR_THICKNESS = 3;
inline constexpr int CURSOR_OFFSET = (OCCUPIED_SIZE - CURSOR_SIZE) / 2;

static_assert(GRID_X + 15 * COLUMN_SPACING + OCCUPIED_SIZE <= 610,
              "SPRITESTEP 16-column matrix must fit the 610px editor");

inline constexpr int cell_x(int column) { return GRID_X + column * COLUMN_SPACING; }
inline constexpr int cell_y(int row) { return GRID_Y + row * ROW_SPACING; }
inline constexpr int centered_offset(int outer, int inner) { return (outer - inner) / 2; }
inline constexpr int empty_offset() { return centered_offset(OCCUPIED_SIZE, EMPTY_SIZE); }

namespace detail {
inline void draw_cursor_flags(Canvas& c, int x, int y, Argb color) {
    // The 41px frame is deliberately mostly open: four short 7px corner flags read clearly over
    // both an occupied 35px cell and a centered 9px empty cell.
    constexpr int L = 7;
    constexpr int T = CURSOR_THICKNESS;
    c.fill_rect(x, y, L, T, color);
    c.fill_rect(x, y, T, L, color);
    c.fill_rect(x + CURSOR_SIZE - L, y, L, T, color);
    c.fill_rect(x + CURSOR_SIZE - T, y, T, L, color);
    c.fill_rect(x, y + CURSOR_SIZE - T, L, T, color);
    c.fill_rect(x, y + CURSOR_SIZE - L, T, L, color);
    c.fill_rect(x + CURSOR_SIZE - L, y + CURSOR_SIZE - T, L, T, color);
    c.fill_rect(x + CURSOR_SIZE - T, y + CURSOR_SIZE - L, T, L, color);
}
}

inline void draw_cursor(Canvas& c, int cellX, int cellY, Argb color) {
    const int x = cellX + CURSOR_OFFSET;
    const int y = cellY + CURSOR_OFFSET;
    detail::draw_cursor_flags(c, x, y, color);
}

inline void draw_empty_cursor(Canvas& c, int emptyX, int emptyY, Argb color) {
    // Empty 9px markers are centered in the same 35px cell coordinate system.
    draw_cursor(c, emptyX - empty_offset(), emptyY - empty_offset(), color);
}
}
