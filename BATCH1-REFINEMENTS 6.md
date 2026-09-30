# SPRITESTEP Batch 1 refinements

Follow-up UI refinements based on the RG40XXH screenshots.

## Pattern
- Occupied steps no longer draw the old 9px outline marker.
- The editing cursor is a thin red outline around the 35px cell and does not cover the cell contents.
- Notes render as two rows inside occupied cells: pitch name on top (for example `A#`) and octave below.
- Compact Pattern footer labels are `I N V P S C A M1 M2 M3 M4 R D`.
- `L+R+D-pad UP/DOWN` changes the selected track's step-duration multiplier, clamped to 1..3.
- `L+R+D-pad LEFT/RIGHT` changes the selected pattern length, clamped to 1..16, and clamps the editing cursor if necessary.
- Steps beyond the pattern length use a darker 9px marker (`#606060`).
- A clearer right-pointing triangle marks the final active pattern column.

## Arrange
- The scene badge was moved left so it cannot overlap the global tempo readout.
- Column hex labels are centered against the same 35px cell geometry as the matrix.
- Tempo is drawn as a global, right-aligned page-header readout.

## Banks
- The `?` action marker is now visible in the left gutter while preserving the shared 37px matrix geometry.

## Navigation MAP
The MAP remains intentionally hidden on ARRANGE/BANKS/PATTERN pending a dedicated compact layout. PROJECT remains part of the navigation contract above ARRANGE/BANKS.
