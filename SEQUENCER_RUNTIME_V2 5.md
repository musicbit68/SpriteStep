# Handheld Sequencer Runtime v2

This milestone continues the Pattern / Banks / Arrange integration and adds the first FMS-style
runtime controls while preserving the project's previously agreed timing model.

## Pattern conditions

Each handheld pattern now has a small per-step condition array alongside its native SPRITESTEP
`PhraseStep` payload. `00` means unconditional. A non-zero byte is encoded as `N/T` in the Pattern
screen: high nibble = occurrence, low nibble = total cycle count. For example `13` means `1/3`.

Conditions use the track's persistent pattern repetition counter. `1/3` fires on pattern cycles
1, 4, 7, ... and the counter continues when the same pattern continues across consecutive Arrange
scenes. Changing to a different pattern resets the counter.

The condition metadata is serialized with the handheld `sequencer` project section and is included in
step cut/paste operations.

## Direction

Each track now has a persistent traversal direction:

- `FORWARD`
- `PINGPONG`
- `REVERSE`
- `RANDOM`

Direction changes which authored step is emitted; it does **not** change the agreed pattern-cycle
duration (`pattern.length * stepDurationMultiplier`). Random traversal is deterministic for a given
track/cycle/ordinal so scheduling and rollback remain reproducible.

## Shuffle

Each track has a `shuffle` byte from `00` to `FF`. Odd-numbered traversal steps are delayed by up to
half of that step's duration. The following even step remains on the original grid, so the pattern's
cycle duration does not grow. `FF` therefore represents the maximum shuffle amount.

This follows FMS's documented behavior: shuffle delays every other step by up to half a step.

## Instrument / Modulation integration

The handheld major-screen ring now reuses SPRITESTEP's existing editors rather than duplicating
their models or DSP controls:

`ARRANGE -> BANKS -> PATTERN -> INSTRUMENT -> MODS -> ARRANGE`

When entering INSTRUMENT or MODS from the handheld Pattern view, the selected step's instrument is
made the current SPRITESTEP instrument. Editing that instrument or its modulation slots therefore
changes the same project data used by handheld playback.

## Timing invariant

The existing invariant remains unchanged:

- one base step = one 16th note;
- each track's step duration is `base_step * stepDurationMultiplier`;
- all tracks share the same absolute audio-frame timeline;
- Arrange scene duration is the longest active pattern cycle;
- pattern cues occur at the end of the current pattern;
- same-pattern continuity across Arrange scenes preserves repetition state.

## Validation

Validated in this milestone:

- native SPRITESTEP CMake build: `libspritestep.a` PASS
- native UI CMake build: `libpt-ui.a` PASS
- sequencer unit tests: PASS
- project serialization round-trip tests: PASS
- Pattern editor tests: PASS
- AddressSanitizer + UndefinedBehaviorSanitizer sequencer tests: PASS
- AddressSanitizer + UndefinedBehaviorSanitizer project-I/O tests: PASS

The PortMaster cross-build was not run in this environment because Docker and the aarch64 cross
compiler are not installed here. The native build is not being presented as an RG40XXH binary test.
