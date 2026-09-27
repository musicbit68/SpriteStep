# SPRITESTEP — Batch 4 cue / FX fixes

## Fixes in this batch

### Pattern ALL^ FX popup
- A now confirms the highlighted FX and closes the persistent ALL^ picker.
- The confirmation is handled before PATTERN dispatch so the underlying Pattern A action cannot reopen the picker.

### BANKS cursor
- A normal D-pad hover no longer fills the hovered pattern cell with `#A0A0A0`.
- The `#A0A0A0` filled box is reserved for an actually playing pattern or the lit phase of a pending cue blink.
- The red cursor outline remains on the hovered cell.

### BANKS cueing
- `A + Left` on a pattern now cues that exact pattern column on all eight tracks. Previously the other tracks were sent their own remembered selected pattern, so the all-column gesture did not launch the intended clips.
- Cue scheduling now rewinds only the affected track's lookahead to the next real pattern boundary and drops/rebuilds audio from that boundary. This makes a cue take effect on the current pattern's next downbeat instead of waiting for the existing lookahead to expire.
- Cue state remains pending for the UI until the real audio clock reaches the launch frame, so all cued cells blink while waiting.
- The cue is consumed after the transport reaches the launch frame; scheduling ahead no longer hides the cue prematurely.
- Each scheduled event now carries its pattern start frame so the next pattern boundary can be calculated from the actual playhead rather than the lookahead cursor.

## Validation

- `pt-ui` CMake target: PASS
- `spritestep` CMake target: PASS
- banks randomization tests: PASS
- pattern editor tests: PASS
- banks view tests: PASS
- arrange view tests: PASS
- sequencer tests: PASS
- SPRITESTEP adapter tests: PASS

No ARM64/PortMaster or RG40XXH hardware build was performed for this batch.
