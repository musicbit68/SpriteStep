# SPRITESTEP Pattern Parameter Fix

This patch fixes Pattern-page parameter editing.

## Fixed
- Occupied Pattern cells now display the **selected parameter value**, not the note name, whenever the selected parameter is not Note.
- The two-line note display is used only while the **Note** parameter is selected.
- Pressing **A** on an empty Pattern cell no longer resets the selected parameter to Note.
- A on an empty cell creates the underlying C4 trigger while preserving the selected footer parameter.
- FX parameters are initialized at 00 when first armed on an empty cell, so their value is visible and can be adjusted with the existing A+D-pad controls.
- The selected parameter remains the active edit target.

## Validation
- `pt-ui` CMake target builds successfully on the host build environment.
- No ARM64/PortMaster build was run.
