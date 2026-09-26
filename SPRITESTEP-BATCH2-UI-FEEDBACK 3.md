# SPRITESTEP Batch 2 UI Feedback

Applied to the Batch 1 refined source after RG40XXH testing.

## Banks / Arrange

- Added visible red cursor outlines to Banks and Arrange matrix cells.
- `L1 + LEFT/RIGHT` now navigates the Banks selector at the bottom and the Arrange page tabs at the bottom.
- Banks no longer has an `X` clear column. `?` remains the randomize action column.
- Banks matrix uses the application background with no occupied-cell background for normal filled patterns.
- Filled Banks patterns use white text; empty pattern slots use dark grey text.
- The current editing/playing pattern uses an `#A0A0A0` box with black text.
- Banks selector uses a white box with black text for the current bank; filled banks are white text and empty banks are grey text, with no normal background.
- Arrange occupied cells use `#A0A0A0` with black text; empty cells remain 9px `#A0A0A0` squares.
- Arrange page tabs now have their own selector state for `L1 + LEFT/RIGHT` navigation.

## Pattern

- Corrected the playhead square so its center aligns with the 9px empty-step square.
- Added the full parameter name above the compact parameter list.
- Parameter order is now:
  `Note, Instrument, Volume, Pan, Slide, Chance, Arpeggio, Filter Frequency, Resonance, Drive, Crush, Downsample, Reverse, Reverb, Delay`.
- `Chance` maps to native `CHA`.
- `Filter Frequency` maps to `CUT`.
- `Resonance` maps to `RES`.
- `Drive` maps to `DRV`.
- `Crush` and `Downsample` edit the high/low nibbles of native `CRU` without changing the underlying FX format.
- `Reverse` maps to native `BCK` playback direction.
- Bare `A` on Pattern focuses Note editing and creates C4 in an empty cell; `A + D-pad` remains the note editor.
- `B` now cuts an occupied step into the step clipboard, or pastes the clipboard step into an empty cell.
- `L1 + B1` highlights the pattern length triangle and track multiplier in red while held.
- Track rate multiplier is constrained to 1–8.
- Conditional trigger cells now show a 15×15 four-quadrant indicator with 2px black outlines; the existing `occurrence/total` condition representation remains unchanged.

## Banks A+B

- `A + B` on Banks cuts the currently playing pattern for the selected track into a pattern clipboard and clears that pattern from the Banks library.

## Validation

- Native CMake build completed successfully through `pt-ui`.
- Pattern editor tests: PASS.
- Banks view tests: PASS.
- Arrange view tests: PASS.
- Changed UI translation units passed standalone C++17 syntax checks.

No ARM64/PortMaster build was run for this batch.
