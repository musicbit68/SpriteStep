# Sequencer Pattern Editing — Wait + Trigless UI

This milestone makes the FMS-style Wait and Trigless metadata directly editable from the handheld Pattern view.

## Pattern parameter strip

The Pattern parameter strip now contains:

`I  NOTE  VOL  PAN  SLI  CHA  ARP  M1  M2  M3  M4  REV  DEL  WAI  TRG  MORE`

- **WAI** — per-step Wait/microtiming delay, stored as PPQN units. `03` is half a step and `06` is one full step at the track's rate.
- **TRG** — per-step trigless state. `ON` means the step evaluates its parameter/FX payload without retriggering the note/voice.

FMS documents Wait as a trigger delay in PPQN units relative to track rate, with 3 = half step and 6 = one whole step. FMS describes trigless steps as changing parameter values without resetting oscillators/envelopes.

## Controls

The existing Pattern editing controls are unchanged:

- **L1 / L-R** selects the parameter.
- **B** increments the selected parameter.
- **A+Left / A+Right** performs fine decrement/increment.
- **A+Up / A+Down** performs coarse decrement/increment.
- **A** on a populated step cuts/stores the complete step, including Condition, Wait and Trigless metadata; on an empty step it pastes the stored step.
- **Delete** clears the selected parameter.

For TRG, the range is 0..1, so the normal increment/decrement controls toggle the state.

## Display

- Wait is shown as two-digit hexadecimal, matching the rest of the Pattern view.
- Trigless is shown as `ON` when enabled and `--` when disabled.
- A step containing only Wait, Trigless, or Condition metadata is still visually treated as an occupied step.

## Timing

Wait changes the event's trigger frame but does not change the pattern's grid duration or the track's cycle duration. The delay is calculated from the track's actual expanded step duration, so the existing global absolute-frame timeline remains authoritative.
