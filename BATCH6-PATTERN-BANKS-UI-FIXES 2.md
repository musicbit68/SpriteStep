# Batch 6 — Pattern cut, ALL FX editing, BANKS resume, and UI polish

## Pattern editing
- A+B on PATTERN now performs a true cut of the current step: the complete step and its condition/wait/trigless metadata are copied to the step clipboard and the source is cleared.
- A+B during Range Edit cuts the entire selected horizontal range to the Range clipboard, including note/instrument/volume/FX/condition/wait/trigless data, then clears the source range.
- The Range Edit outline is now 3px thick (2px thicker than the previous 1px outline).

## ALL^ FX parameter editing
- B opens the ALL^ grouped FX picker and B closes it.
- Closing the picker remembers the selected FX command.
- A is reserved for editing the selected FX value on the Pattern page.
- A+direction changes the selected FX value without reopening the picker.
- If the selected FX is absent from a step, the edit path creates an FX slot for it.

## BANKS → PATTERN → START behavior
- When a BANKS cue launches a pattern on one track, that selected pattern is also remembered as that track's current BANKS selection.
- When an all-track column cue launches, all eight selected-pattern memories are updated to the cued column.
- This keeps BANKS and PATTERN aligned after stopping playback and pressing START again, instead of falling back to the previous per-track selection.

## Navigation map
- Mixer and Effects bars are moved closer to the main/context blocks.
- Map height is reduced so the whole map can sit lower on the 640x480 display, avoiding the Pattern row-8 / columns-15-16 overlap.

## ARRANGE footer
- The bottom page numbers now use the same 35px box, 10px text offset, and 31px spacing as the BANKS bottom bank selector.

## Validation
- pt-ui build: PASS
- Pattern editor tests: PASS
- Banks view tests: PASS
- Arrange view tests: PASS
- Sequencer tests: PASS
- SPRITESTEP adapter tests: PASS
- No ARM64/PortMaster build was run in this batch.
