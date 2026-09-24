# SPRITESTEP — Playhead / Cue / Arrange Visualization v7

This revision makes the handheld transport visualization substantially more explicit while keeping it separate from the sequencer/audio model.

## Pattern View
- Every step in the active pattern receives a small ~9px square marker.
- Empty/resting steps use the dim empty marker; authored steps use the playhead accent.
- The currently sounding step grows to 13px, is filled, and receives a 1px outer outline.
- The marker follows the per-track runtime playhead, so all eight tracks can move independently.
- The drawing is gated by the actual handheld transport `isPlaying` state.

## BANKS
- The playing bank/pattern is indicated by dark text inside a light box.
- Runtime bank/pattern is reported even when a pattern has no sounding event at the current frame.
- A queued pattern continues to blink until the cue is consumed at the pattern boundary.
- The playing indicator takes priority over the cue indicator.

## ARRANGE
- The active scene column remains inverted in the scene header and in populated scene cells.
- Empty cells in the active scene column receive the playhead marker.
- The header also displays an explicit `SCxx` scene readout.
- Scene readback comes directly from the sequencer runtime rather than the most recent scheduled event.

## Transport readback fixes
`Host::playheads()` now reports runtime scene/bank/pattern independently of event history and only overlays the current step when a real scheduled event exists.

`Sequencer::playing_scene_at()` now returns the authoritative runtime scene while transport is running rather than deriving it from event history.

## Tests
Passed locally:
- sequencer tests
- pattern editor tests
- BANKS view tests
- ARRANGE view compilation
- `songcore::Host` header syntax compilation

The ARM64 PortMaster build still needs to be produced by GitHub Actions after this source revision is pushed.
