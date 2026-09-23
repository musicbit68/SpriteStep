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

static_assert(GRID_X + 15 * COLUMN_SPACING + OCCUPIED_SIZE <= 610,
              "SPRITESTEP 16-column matrix must fit the 610px editor");

inline constexpr int cell_x(int column) { return GRID_X + column * COLUMN_SPACING; }
inline constexpr int cell_y(int row) { return GRID_Y + row * ROW_SPACING; }
inline constexpr int centered_offset(int outer, int inner) { return (outer - inner) / 2; }
inline constexpr int empty_offset() { return centered_offset(OCCUPIED_SIZE, EMPTY_SIZE); }
}
