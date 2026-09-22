# Native Pattern / Banks / Arrange integration v1

This milestone integrates the handheld sequencer views into SPRITESTEP's native C++ UI and transport.

## Screens

- `PATTERN`
- `BANKS`
- `ARRANGE`

The persistent data remains `songcore::Project::sequencer` and is shared by the UI and sequencer runtime.

## Navigation

On the new sequencer screens:

- D-pad moves within the active view.
- `R + LEFT/RIGHT` cycles ARRANGE → BANKS → PATTERN.
- `L + LEFT/RIGHT` changes the Pattern parameter selector.
- `START` starts/stops the native handheld Arrange transport.

Legacy SPRITESTEP screens retain their existing navigation behavior.

## Pattern

- 8 tracks × 16 steps.
- Pattern length is 1–16.
- Track number at left is the step-duration multiplier.
- Pattern data uses SPRITESTEP `PhraseStep` / native FX semantics.
- `A` cuts an occupied step into the temporary step clipboard, or pastes into an empty step.
- `B` edits the selected parameter immediately.
- `A + D-pad` performs normal parameter editing.
- `L + LEFT/RIGHT` changes the displayed parameter.

## Banks

- 8 banks × 16 patterns per track.
- `B + LEFT` performs the track-local BANK operation.
- `A + LEFT` performs the all-track BANK operation.
- Pattern cues remain track-local; column cues use the selected pattern column.

## Arrange

- 8 tracks.
- 16 scenes per page.
- 8 pages = 128 scene addresses.
- Cells store a two-digit bank/pattern macro.
- Pattern data itself remains in the track-local Banks library.

## Transport

The native handheld sequencer is connected to the existing SPRITESTEP audio router through
`SPRITESTEPAdapter`. Its scheduler produces exact audio-frame events and routes each `PhraseStep`
through SPRITESTEP's existing FX/state handling.

The legacy Song/Chain scheduler and the new handheld scheduler are mutually exclusive during transport.

## Tests

Passed:

- pattern editor tests
- banks view tests
- arrange view tests
- sequencer timing tests
- sequencer project IO round-trip tests
- all of the above under AddressSanitizer + UndefinedBehaviorSanitizer
- full `spritestep` static library build
- full `pt-ui` static library build
