# SPRITESTEP Batch 4 — UI / scale / Banks / Pattern changes

## Global UI
- Replaced the text navigation MAP with the compact block MAP on every page.
- Current page is `#A0A0A0`; other pages are dark empty blocks.
- Removed the `T>...` tempo readout from the right side of the UI.
- Shared matrix cursor is now a 41px cursor with 3px corner flags.
- Arrange and Banks footer selectors are centered.

## Project tempo
- The full songcore Project owns the tempo. The handheld sequencer adapter now copies that tempo into the SPRITESTEP sequencer when playback starts and when the scheduling window is refreshed.
- This fixes the handheld transport continuing to use the sequencer's old default 120 BPM.

## Banks
- D-pad never enters the Bank-number footer. Bank changes are L1 + Left/Right.
- A held on Banks outlines the whole selected column; B remains the single-track modifier for B + Left/Right operations.
- `B + Left` on `?` randomizes one track; `A + Left` randomizes all tracks.
- `B + Left` on a pattern cues that track; `A + Left` cues the whole pattern column.
- Cue markers blink at a 500ms cycle until the pattern starts, then the playing pattern has a solid A0/black box.
- Random generation now chooses note occupancy separately, leaving genuinely empty steps, and generated notes are snapped to the Project Scale.

## Pattern
- Pattern view follows the pattern actually playing on Banks while the transport is running.
- Pattern note insertion/editing uses the Project Scale.
- Footer is reduced to `N I V P S C A ALL^`.
- `ALL^` opens the grouped PocketTracker FX picker. B commits the selected FX and closes the picker.
- Direction and Shuffle controls are shown in the header for the selected track. L1 + Up/Down selects the direction/shuffle control; A + Left/Right/Up/Down edits it.
- L1 + R1 highlights the track rate multiplier and pattern end indicator in red while held.
- Track rate remains 1–8.
- The Pattern playhead is centered on the 9px empty-step marker.

## Arrange
- Occupied cells remain A0 with black text and empty cells remain the 9px matrix markers.
- The scene-page footer is centered and the current page is shown as the white/black selector.

## Validation
- Native `pt-ui` build completed successfully.
- Banks randomization tests: PASS.
- Pattern editor tests: PASS.
- Banks view tests: PASS.
- Arrange view tests: PASS.
- Sequencer tests: PASS.
- SPRITESTEP adapter tests, including a project-tempo assertion at 60 BPM: PASS.
- Full SDL desktop build was not run because this environment cannot fetch SDL2 from GitHub.
- No ARM64/PortMaster build was run for this batch.
