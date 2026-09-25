# Playhead / Cue / Arrange visualization v6

This build continues the handheld sequencer UI work and wires the transport readback into all three major sequencer views.

## Pattern View
- Reads all eight track playheads each frame.
- Every active step gets a small ~9 px outlined playhead marker.
- The currently sounding step grows to ~13 px with a thicker outline.
- The marker is positioned at the bottom of the step cell, leaving the parameter value untouched.
- Pattern View therefore shows independent track playheads simultaneously rather than only the selected track.

## BANKS
- The currently playing pattern is rendered as dark text on a light/inverted box.
- A queued pattern blinks between normal text and the same inverted treatment until it starts.
- The old `+` cue marker is removed.
- The BANK selector retains its existing cursor behavior.

## ARRANGE
- The playing scene is indicated in the scene-number header with an inverted box.
- The active scene's pattern macro cells are also inverted so the playing scene is visible as a column.
- Empty cells in the playing scene receive the playhead marker.
- The old per-row `>` marker is removed; scene playback is represented consistently as a scene/column indicator.
- If the playing scene is outside the currently displayed 16-scene page, no false indicator is drawn.

## Transport wiring
`InputDispatcher::set_now()` now refreshes:
- `AppState::isPlaying`
- all eight handheld playheads
- all eight handheld cue states
- the handheld playing scene
- the shared blink phase

This keeps Pattern, BANKS, ARRANGE, and SONG using the same frame-level transport snapshot.

## Validation
- Full native CMake Debug build: PASS (`libspritestep.a`, `libpt-ui.a`).
- Pattern editor standalone tests: PASS.
- BANKS standalone tests: PASS.
- ARRANGE standalone tests: PASS.
- No ARM64/Knulli/RG40XXH hardware build was performed in this environment.

The visual behavior follows the FMS reference model: FMS documents a main grid playback indicator and specifies that queued pattern slots blink until the transition occurs. See https://lo-bit.club/fms/guide, especially the Grid and Data view sections.
