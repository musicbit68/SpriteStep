# SPRITESTEP v4 fixes

This source package is based on SPRITESTEP v3 and fixes the build issues found in the Android build log.

## Fixes

- **Android `PT_UI_REVISION` build error fixed.** The Android `spritestep-sdl` shared-library target now receives the same `PT_UI_REVISION` definition as the desktop/PortMaster target, so `shell/app.cpp` can compile its SPRITESTEP boot banner on Android.
- **Help-screen `-Wswitch` warning fixed.** The new SPRITESTEP `PATTERN`, `BANKS`, and `ARRANGE` screen types are now handled by the help-screen fallback. They reuse the existing Phrase/Song help topics until dedicated help topics are introduced.
- **PortMaster provenance updated.** The packaged `build-info.txt` identifies this source package as v4 while retaining the UI revision marker `SPRITESTEP-UI-V3`.

## Verification performed

- `bash -n shell/build-portmaster.sh`
- Confirmed both desktop/PortMaster and Android `spritestep-sdl` CMake targets define `PT_UI_REVISION`.
- Confirmed `help_screen_topic()` handles all current `ScreenType` values, including PATTERN/BANKS/ARRANGE.
- Confirmed the PortMaster workflow already packages `build/portmaster/spritestep.zip` and uses the `spritestep-build` cross-build container.

A full Android/ARM64 release build was not performed in this environment.
