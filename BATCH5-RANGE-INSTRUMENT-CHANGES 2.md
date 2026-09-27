# SPRITESTEP Batch 5 — Range Edit + track-consistent Bank randomization

## Range Edit

Added an FMS-inspired horizontal Range Edit mode to PATTERN. FMS documents Range Edit as selecting a range of steps and then editing/copying/pasting that range; while active the D-pad extends or subtracts the range, and value changes can operate on the selected steps.

SPRITESTEP mapping:

- `L+B` on PATTERN enters Range Edit at the current step.
- `D-pad LEFT/RIGHT` moves the active range edge while Range Edit is active.
- The anchor stays fixed; the inclusive span between anchor and edge is the range.
- `B` cancels Range Edit.
- `A + LEFT/RIGHT/UP/DOWN` applies the currently selected PATTERN footer parameter to every step in the range.
- `A+B` applies the selected parameter's normal delete/reset action to every step in the range.
- `L+A` copies the selected range, including note/instrument/volume/FX/condition/wait/trigless data.
- `L+A` outside Range Edit pastes the copied range starting at the current step.
- Pasting is clipped at the pattern end.
- A range is horizontal within one track/pattern. Moving UP/DOWN exits Range Edit and changes track.
- The selected parameter still uses each step's normal `CursorContext`, so note edits remain Project Scale-aware and existing FX/value clamp/wrap rules are preserved.
- `ALL^` remains a picker and is not treated as a numeric range-edit parameter.

The range is rendered with red outlines around the selected cells while the active edge keeps the normal 41px Pattern cursor.

## BANKS randomization instruments

Bank randomization now receives one instrument ID per track from the UI.

Selection rule:

1. Prefer an instrument already used by a note in that track's existing SPRITESTEP patterns.
2. If the track has no existing instrument usage, choose the first configured/non-free instrument in the project instrument pool.
3. If the entire project has no configured instruments, fall back to instrument 0.

Every generated note in the randomized pattern for that track uses the selected instrument. Empty generated steps remain empty.

This means randomization no longer generates arbitrary instrument IDs per note, while different tracks can still retain different instruments.

## Validation

- `pt-ui` CMake target: PASS
- `banks randomization tests`: PASS
- `pattern editor tests`: PASS
- `banks view tests`: PASS
- `arrange view tests`: PASS
- `sequencer tests`: PASS
- `sequencer project IO tests`: PASS
- `SPRITESTEP adapter tests`: PASS

No ARM64/PortMaster build or RG40XXH hardware test was performed for this batch.
