# SPRITESTEP source package — rebrand + sequencer visualization v1

This package is based on the Playhead/Cue/Arrange v6 source tree and prepares it for the next GitHub push under the working project name **SPRITESTEP**.

## Included

- Pattern View playhead visualization: active-step squares grow from about 9px to about 13px on the current step.
- BANKS playing-pattern indicator: the currently playing pattern is shown as dark text on a light box.
- BANKS cue indicator: a queued pattern blinks until the cue is consumed.
- ARRANGE scene indicator: the active scene is shown in the header and active scene cells are highlighted.
- Transport wiring for all 8 track playheads, 8 track-local cues, playing scene, and blink phase.
- Existing Pattern / Banks / Arrange sequencer implementation from the v6 source package.
- PortMaster packaging renamed to SPRITESTEP so it installs under `ports/spritestep/` and uses `spritestep.aarch64`.
- SDL desktop target renamed from `pockettracker-sdl` to `spritestep-sdl`.
- Native sequencer library target renamed from `pockettracker` to `spritestep`.
- PocketTracker adapter filenames renamed to `spritestep_adapter.*`.
- Android package namespace/application ID renamed to `com.conanizer.spritestep` and the visible app name changed to SPRITESTEP.
- SPRITESTEP_HOME replaces POCKETTRACKER_HOME for the new data-root name.

## Important

The GitHub repository itself can remain named `musicbit68/pockettracker` for now. The CI workflow builds whatever is committed to that repository, so these source changes must be committed and pushed before the Actions PortMaster job will contain them.

## Verification performed in this environment

- `bash -n shell/build-portmaster.sh` — PASS
- `bash -n shell/portmaster/SPRITESTEP.sh` — PASS
- `sh -n shell/miyoo/launch.sh` — PASS
- Native CMake configure — PASS
- Native core/UI build — PASS (`spritestep` and `pt-ui` targets)
- Dependency-light sequencer tests — PASS
- Sequencer project persistence tests — PASS

The full SDL desktop shell configure could not complete here because SDL2 was not installed and this environment cannot reach GitHub to download it. The aarch64 PortMaster build was not run here; GitHub Actions should perform that build after the push.
