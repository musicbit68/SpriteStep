# SPRITESTEP Batch 1 — layout foundation

Implemented from the Batch 1 UI list:

- Removed the visible 8-track note monitor from the shared right rail.
- Moved `T>BPM` into the page header area.
- Added PROJECT to the visible navigation path above ARRANGE/BANKS and made R+UP/R+DOWN enter/leave PROJECT for those columns.
- Added shared sequencer matrix geometry in `native/ui/matrix_geometry.h`:
  - 37px column spacing
  - 37px row spacing
  - 35px occupied cells
  - 9px empty cells
  - empty-cell color `#A0A0A0`
- ARRANGE, BANKS and PATTERN now use the shared matrix coordinates.
- ARRANGE expanded to the full 610px editor width so all 16 columns fit the 37px grid.
- PATTERN empty steps now render as the shared 9px marker rather than a full cell.
- BANKS uses the common 35px matrix cells.
- The legacy navigation map is hidden on the three full-width matrix screens for now so it cannot cover the new matrix. Its placement will be refined with the MAP work rather than reintroducing a matrix clip.

Validation:

- Syntax-only compilation passed for navigation map, Arrange, Banks, Pattern and layout sources.
- PortMaster build script shell syntax passed.
- Full ARM64/Android builds were not run in this environment.
