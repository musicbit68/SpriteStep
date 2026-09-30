# SpriteStep Pattern Fixes — 2026-09-27

This source tree is based on the clean Batch 7 SpriteStep source, not the accumulated/duplicated batch ZIP.

Implemented:
- Ping-pong playback now gives each directional traversal the full pattern length, repeating the terminal step at the turn (16-step example: 14,15,16,16,15,14).
- Pattern speed/rate, Direction, and Shuffle are stored per Pattern.
- Legacy track-level speed/Direction/Shuffle are migrated into Pattern data when loading old projects.
- Pattern header uses `TRA1 BAN0 PAT3`.
- Direction and Shuffle moved to the Pattern footer as `DIR` and `SHF xx`; L+D-pad UP/DOWN moves focus between footer controls.
- Pattern speed continues to use L+R+D-pad, but edits the selected Pattern rather than the Track.
- SELECT+A on the Pattern screen randomizes the selected footer parameter; Range Edit randomizes each occupied/selected step independently. Empty steps may receive a scale-snapped note when randomization needs a playable step.
- Range Edit value operations skip truly empty steps instead of creating parameter-only data in rests.
- Existing Pattern ALL^ picker behavior is preserved.

Verification performed:
- native sequencer tests: PASS
- sequencer project I/O/migration tests: PASS
- Pattern editor tests: PASS
- native CMake build (`spritestep` + `pt-ui`): PASS
- PT_UI_REVISION is explicitly defined for both Android/native and desktop shell CMake targets.

Desktop shell configure/build could not be completed in this environment because SDL2 was not installed and the environment has no network access to fetch it. The CMake source was inspected and both shell targets define `PT_UI_REVISION`.
