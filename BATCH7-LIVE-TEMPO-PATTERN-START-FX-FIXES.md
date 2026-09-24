# SPRITESTEP Batch 7 — Live Tempo, Pattern START, FX Picker Close

## Changes

### 1. Tempo changes while the handheld sequencer is running

The handheld scheduler now observes the current project tempo on every scheduling pass.

When the project tempo changes during playback:

- future handheld audio events already queued at the old tempo are cleared;
- the current runtime step boundary is preserved;
- scheduling resumes from that boundary using the new tempo;
- the currently sounding step is not retriggered;
- no Stop/Start is required.

This applies to both BANKS/live-pattern playback and ARRANGE playback.

`SPRITESTEPAdapter::schedule_until()` also refreshes the sequencer tempo from the audio project so direct adapter scheduling cannot retain a stale tempo.

### 2. START on PATTERN

START on PATTERN now launches the selected handheld pattern set using the same BANKS/live-pattern transport as the BANKS page.

Previously PATTERN fell through to ARRANGE playback, which made START appear not to work as expected on the Pattern page.

STOP behavior remains unchanged.

### 3. ALL^ FX popup close

The persistent Pattern ALL^ FX picker now handles B before generic overlay swallowing.

Behavior is now:

- **B** — open the ALL^ FX picker
- **B** — close the picker and remember the highlighted FX
- **A** — edit/adjust the selected FX value

The selected FX remains remembered after closing and reopening the picker.

## Tests

Native build:

- `pt-ui` — PASS

Direct tests:

- pattern editor tests — PASS
- banks view tests — PASS
- arrange view tests — PASS
- sequencer tests — PASS
- banks randomization tests — PASS
- sequencer project IO tests — PASS
- SPRITESTEP adapter tests — PASS

The sequencer tests include a live-tempo regression test. The adapter tests verify that changing the audio project's tempo while the adapter is already running updates its timing model.

## Hardware status

No RG40XXH/PortMaster hardware build or hardware test was performed as part of Batch 7. The source was built and tested natively on the development environment.
