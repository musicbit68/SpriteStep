# SpriteStep sprites

SpriteStep instruments now have optional visual sprite metadata.

- `spriteId = -1` means **NO SPRITE**.
- `spriteId = 0..127` selects one of the 128 built-in sprites.
- The source artwork is a transparent **256×512 PNG**, 8 columns × 16 rows, with **32×32 source tiles**.
- The RG40XXH runtime representation is a 16×16 indexed version drawn at 2×, using a 256-color palette, to keep Pattern-page rendering inexpensive while retaining hard pixel edges.
- The sprite assignment is stored on the Instrument, not on PatternStep. All occurrences of the same instrument therefore share the same visual identity.
- Old projects remain compatible because the `spriteId` JSON field is optional and defaults to `-1`.

## Picker controls

On **INST.POOL**, with the cursor on the instrument name column:

- **L + A** — open Sprite Picker
- **A + UP** — previous sprite
- **A + DOWN** — next sprite
- **A + B** — randomly choose a sprite (excludes NO SPRITE)
- **B** — assign the centered sprite
- **L** — cancel

The picker is a vertical reel. The centered sprite is the selection; half of the previous and next sprites remain visible above and below it.

## Pattern page

The footer parameter order is now:

`I  N  V  P  S  C  A  ALL^`

When **Instrument** is selected, a cell uses the instrument's assigned sprite instead of its hexadecimal instrument number. After an Instrument value is edited, the selected cell remains numeric briefly so the new value can be seen before the sprite display returns.

A `NO SPRITE` instrument always remains hexadecimal.

## Current artwork

The included `docs/internal/images/spritestep_sprites_128.png` is the first integrated prototype based on the SpriteStep moodboard. The renderer deliberately uses sprite IDs, so the artwork can be replaced later without changing project data or UI code.
