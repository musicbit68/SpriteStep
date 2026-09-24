
## Pattern parameter selection fix

Fixed a Pattern-page regression where pressing **A** always reset `seqPatternParameter` to `NOTE`.
The selected footer parameter now survives an A press, so L1+Left/Right selection remains active.
On an empty cell, A still creates the default C4 note, but it no longer changes the selected parameter.
