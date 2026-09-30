# SpriteStep

SpriteStep is a new C++17 step-sequencer/groovebox architecture targeting the Anbernic RG40XXH.

## First milestone

This initial engine milestone intentionally has no graphical UI. It establishes the musical model and tests the behaviors that the UI and audio engine will build on:

- 8 independent tracks.
- 8 banks × 16 patterns = 128 pattern slots per track.
- Patterns are 1–64 steps.
- Every step has independent values for every defined parameter; unset values are represented explicitly as `--` (`ParameterValue::set == false`).
- Instrument defaults resolve whenever a step parameter is unset.
- One shared master transport supports polymetric pattern lengths.
- Forward, reverse, and ping-pong pattern playback.
- Quantized pattern launch at the next global bar boundary.
- Legato launch on the next step without resetting transport position.
- Headless C++ tests.

The audio engine is deliberately only an interface at this stage. The next milestone can add miniaudio plus the voice/sample/DSP layer without changing the sequencer model.

## Build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/SpriteStep
```

## Design notes

The master transport never belongs to a Pattern. A TrackPlayer maps the continuous transport position into each active Pattern independently. This is what allows, for example, a 13-step bass pattern and a 16-step drum pattern to loop against the same musical clock.

Pattern switching changes the active Pattern reference, not the master transport. In a future audio/UI layer, legato switching will use the continuous transport position to resolve the newly selected pattern's local step.
