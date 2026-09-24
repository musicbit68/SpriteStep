# Handheld Sequencer Runtime v3

This milestone adds the next FMS-style timing/trigger features and connects the handheld transport
readback to the existing SPRITESTEP UI.

## FMS-style Wait / microtiming

Each pattern step now has `wait_ppqn` metadata. The value is a trigger delay relative to that track's
step duration:

- `00` = no additional delay
- `03` = half a step
- `06` = one full step
- larger values are retained as additional PPQN delay values

The conversion is `wait_ppqn * stepDuration / 6`. This follows the FMS guide's definition of Wait in
PPQN units, where 3 equals half a step and 6 equals a whole step.

Wait changes the trigger frame only. The track's underlying grid, cycle duration, pattern repetition
counter, and shared absolute audio-frame clock are unchanged.

## Trigless steps

Each pattern step now also has a `trigless` flag. A trigless step still passes its SPRITESTEP
`PhraseStep` payload through the native scheduler, including FX/state changes, but its note is converted
to `Note::EMPTY()` at the adapter boundary. SPRITESTEP already treats an empty-note step as an
FX/parameter-only step, so no new note-on is emitted and the currently sounding voice is not
retriggered.

This is deliberately separate metadata rather than a special value inside `PhraseStep`, preserving
SPRITESTEP's native step format.

## Playhead readback

The handheld sequencer now keeps a bounded history of the actual scheduled trigger frames. The UI reads
the latest event at or before the audio frame instead of reading the scheduler's lookahead cursor.
This matters because the handheld scheduler intentionally queues future events ahead of the sound.

The existing UI `PlaybackPosition` carrier is reused for the handheld Pattern / Arrange screens:

- `phraseId` = pattern
- `phraseStep` = authored step
- `chainId` = bank
- `chainRow` = pattern
- `songRow` = Arrange scene

Pattern View now receives its playhead directly, and Arrange View gets a live scene marker.

## Pattern cue visualization

BANKS now reads the live cue state from the handheld sequencer. A `+` is drawn beside a queued target
pattern while it is still waiting for the current pattern boundary.

The existing BANKS gestures now call the handheld transport cue API:

- B cue = one track
- A cue = selected pattern column across all tracks
- randomize / clear remain editing operations

The cue remains runtime-only and is not serialized.

## Persistence

The sparse handheld project serialization now stores:

- `conditions`
- `waitPPQN`
- `trigless`
- track duration multiplier
- direction
- shuffle
- Arrange scenes

The serialized handheld sequencer schema version is now `2`. Older projects with no handheld
`sequencer` section continue to load with default values.

## Validation

Validated after this milestone:

- native SPRITESTEP CMake build: `libspritestep.a` PASS
- native UI CMake build: `libpt-ui.a` PASS
- sequencer unit tests: PASS
- project serialization round-trip tests: PASS
- SPRITESTEP adapter tests: PASS
- AddressSanitizer + UndefinedBehaviorSanitizer sequencer tests: PASS
- AddressSanitizer + UndefinedBehaviorSanitizer project-I/O tests: PASS

The PortMaster/aarch64 build was not run in this environment because Docker and an aarch64 cross
compiler are not installed here. This package is source/development state, not a verified RG40XXH
binary.
