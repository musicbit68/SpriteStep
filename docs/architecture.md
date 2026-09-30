# SpriteStep Architecture

## Core model

`Project -> Tracks[8] -> Banks[8] -> Patterns[16] -> Steps[64]`

The 8×16 bank layout gives each track 128 pattern slots while retaining the bank/pattern addressing used by the intended UI and future Song/Performance mode.

## Step parameters

A Step contains a value for every parameter. A parameter can be unset (`--`) or explicitly set to a byte value. This is intentionally different from a three-FX-slot model: all parameters may coexist on the same step.

At note trigger time, an unset step parameter resolves to the Instrument default. An explicit step value overrides that default for that note.

## Timing

The Transport is global and independent from patterns. Each TrackPlayer maps transport ticks to its active Pattern. Pattern length therefore does not define the global bar length.

This supports polymeter such as 16/13/24/7-step patterns sharing one transport.

## Pattern launch

- Quantized: pending pattern becomes active at the next global downbeat.
- Legato: pending pattern becomes active on the next step, preserving the absolute transport position. The new pattern is not restarted at step 1.

## Audio boundary

The Sequencer produces events. The AudioEngine will resolve Instrument defaults and Step overrides, then create/manage voices. The audio callback must not wait on UI locks or perform filesystem I/O.

## Future layers

- miniaudio device/output
- sample loading and slicing
- envelopes, LFOs, filters, pitch processing
- voice manager and effects
- JSON project persistence
- graphical UI
- Song/Performance mode referencing existing track-specific patterns
- RG40XXH packaging
