# SPRITESTEP original Batch scope audit

This audit compares the current source against the Batch 1 / Batch 1 refinement / Batch 2 / Batch 4 scopes carried in the source documentation and the later cue/FX fixes.

## Global UI — covered

- Right-side 8-track note monitor removed.
- Tempo moved into the page/header area and removed from the old right rail.
- PROJECT is part of the visible navigation hierarchy above ARRANGE/BANKS.
- ARRANGE, BANKS and PATTERN use the shared matrix geometry.
- Compact MAP is present on the visible pages.
- Shared sequencer cursor is 41px with 3px corner flags.

## PATTERN — covered

- Two-line note display.
- 1–8 track step-duration multiplier.
- 1–16 pattern length with END triangle.
- Darkened steps beyond pattern length.
- Project Scale-aware note insertion/editing.
- Current Banks-playing pattern is followed while transport is running.
- Compact footer: `N I V P S C A ALL^`.
- Parameter-name readout above the footer.
- Native FX mappings remain available, with `ALL^` opening the grouped FX picker.
- Direction and Shuffle controls in the header.
- L+R rate/length highlight behavior.
- Pattern playhead markers.
- Condition, Wait and Trigless metadata remain supported.
- Single-step B cut / paste remains supported.
- A+B reset/delete semantics remain supported.
- **Batch 5 adds FMS-inspired Range Edit.**

## BANKS — covered

- `?` randomizer column; X clear column removed from the handheld UI.
- D-pad stays in the 8×16 pattern matrix; bank selection uses L+LEFT/RIGHT.
- B acts on one track; A acts on the whole pattern column for cue/randomize operations.
- A-held whole-column cursor.
- Normal hover is only the red cursor; it does not create an A0 filled box.
- Playing pattern uses A0/black inverted cell.
- Pending cues blink at a 500ms cycle and remain pending until the actual launch frame.
- A+pattern cues the same column across all tracks.
- Randomizer chooses occupancy separately from note generation.
- Randomized notes follow Project Scale.
- **Batch 5 keeps one existing instrument per track for generated notes.**
- A+B cuts the currently playing pattern into the pattern clipboard.

## ARRANGE — covered

- Renamed/used as the linear arrangement view.
- Scene/page navigation and centered page selector.
- Shared matrix geometry and A0 occupied cells.
- Scene/playback indicator.
- Arrange runtime remains separate from Banks live performance playback.
- Conditional pattern repetition semantics remain in the native sequencer runtime.

## PLAYBACK / TRANSPORT — covered

- Project tempo is copied into the handheld sequencer adapter.
- Runtime direction: forward, ping-pong, reverse, random.
- Per-track shuffle and step-duration multiplier.
- Wait/microtiming and trigless handling.
- Eight track playheads and cue state are transported to the UI.
- Bank cues launch at actual pattern boundaries rather than at an arbitrary lookahead boundary.

## Items intentionally superseded by later batches

- The early Batch 1 footer list (`I N V P S C A M1 M2 M3 M4 R D`) was superseded by the later compact `N I V P S C A ALL^` design.
- The earlier 37px/35px matrix cursor was superseded by the 41px cursor while preserving the shared matrix coordinates.
- The initial hidden MAP on matrix pages was superseded by the compact MAP shown on all pages.

## Remaining validation gap

The source and native desktop UI targets are validated in the current environment, but the current Batch 5 source has not yet been cross-built for ARM64/PortMaster or installed/tested on the RG40XXH.
