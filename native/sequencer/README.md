# Sequencer core prototype

This directory is the first headless sequencing layer for the new 8-track Pattern / Banks / Arrange workflow.

## Timing model

- Every Pattern contains 1–16 steps.
- The base clock is SPRITESTEP's existing 16th-note `frames_per_step()` timing.
- A Track's displayed speed value is a step-duration multiplier:
  - `1` = one 16th note per pattern step
  - `2` = two 16th notes per pattern step
  - `3` = three 16th notes per pattern step
  - etc.
- Tracks share one absolute audio-frame timeline; they do not own independent clocks.
- Pattern cycle duration is `pattern.length * track.step_duration_multiplier` base steps.
- Arrange scene duration is the longest effective pattern duration among its active tracks.
- Shorter tracks repeat their patterns until the scene boundary.
- Bank/Pattern cues are track-local and take effect at the end of the currently playing Pattern.

## SPRITESTEP compatibility

`PatternStep` is an alias of `songcore::PhraseStep`, so the new Pattern UI can expose the existing SPRITESTEP note, volume, instrument, and three FX slots without creating a second incompatible FX representation.

## Current tests

`sequencer_tests.cpp` is a dependency-light host test executable. It currently verifies:

1. Pattern cycle duration at 1x/2x.
2. Different track speeds sharing one global clock.
3. Short Patterns.
4. Arrange scene duration based on the longest effective Pattern.
5. Pattern cueing at a Pattern boundary.

Build manually from `native/` with a C++17 compiler:

```sh
c++ -std=c++17 -I native native/sequencer/sequencer.cpp native/sequencer/sequencer_tests.cpp -o sequencer_tests
./sequencer_tests
```

## Persistent project model

The Pattern / Bank / Arrange data is now part of `songcore::Project` as `Project::sequencer`.
The sequencer implementation aliases that data rather than keeping a duplicate project graph.
This keeps one live project object for the UI, file I/O, and playback layer.

The `.ptp` JSON adds a top-level `sequencer` object only when sequencer data has been authored. Inside
it, patterns are stored sparsely by `{bank, pattern}`; each authored pattern contains its length and
all 16 SPRITESTEP-compatible `PhraseStep` values. Arrange scenes store eight track-local pattern
references. Runtime playhead, repetition, and cue state remain transient and are never serialized.

Legacy `.ptp` files without `sequencer` continue to load with an empty/default sequencer. The existing
SPRITESTEP project schema is otherwise untouched when the new sequencer has never been used.

The persistence test is `sequencer_project_io_tests.cpp`:

```sh
c++ -std=c++17 -I native native/sequencer/sequencer_project_io_tests.cpp -o sequencer_project_io_tests
./sequencer_project_io_tests
```
