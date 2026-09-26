#pragma once
#include "ui/canvas.h"
#include "ui/sprite_catalog.h"
namespace pt::ui {
inline void draw_sprite(Canvas& c, int spriteId, int x, int y) {
    if (spriteId < 0 || spriteId >= SPRITE_COUNT) return;
    const uint32_t first = SPRITE_RUN_OFFSETS[static_cast<size_t>(spriteId)];
    const uint32_t last  = SPRITE_RUN_OFFSETS[static_cast<size_t>(spriteId + 1)];
    for (uint32_t i = first; i < last; ++i) {
        const SpriteRun& r = SPRITE_RUNS[i];
        c.fill_rect(x + static_cast<int>(r.x) * SPRITE_SCALE,
                    y + static_cast<int>(r.y) * SPRITE_SCALE,
                    static_cast<int>(r.len) * SPRITE_SCALE, SPRITE_SCALE,
                    SPRITE_PALETTE[r.color]);
    }
}
inline void draw_sprite_clipped(Canvas& c, int spriteId, int x, int y, int clipX, int clipY, int clipW, int clipH) {
    Canvas::ClipScope clip(c, clipX, clipY, clipW, clipH);
    draw_sprite(c, spriteId, x, y);
}
}
