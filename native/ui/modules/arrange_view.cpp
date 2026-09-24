#include "ui/modules/arrange_view.h"

#include "ui/helpers.h"
#include "ui/matrix_geometry.h"

#include <algorithm>

namespace pt::ui {

namespace {
constexpr int LEFT = 2;
constexpr int TOP = matrix::GRID_Y;
constexpr int BANK_LABEL_Y = 2;
constexpr int FOOTER_Y = 350;

std::string macro_text(const songcore::SequencerPatternRef& ref) {
    if (!ref.active) return "  ";
    return songcore::hex2((static_cast<int>(ref.bank) << 4) | static_cast<int>(ref.pattern));
}
} // namespace

int ArrangeViewModule::absolute_scene(int page, int column) {
    if (!valid_page(page) || !valid_column(column)) return -1;
    return page * SCENES_PER_PAGE + column;
}

void ArrangeViewModule::ensure_scene(songcore::SequencerData& data, int scene) {
    if (scene < 0 || scene >= MAX_SCENES) return;
    while (static_cast<int>(data.scenes.size()) <= scene)
        data.scenes.emplace_back();
}

int ArrangeViewModule::macro_value(const songcore::SequencerPatternRef& ref) {
    if (!ref.active) return -1;
    return (static_cast<int>(ref.bank) << 4) | static_cast<int>(ref.pattern);
}

songcore::SequencerPatternRef ArrangeViewModule::ref_from_macro(int value) {
    songcore::SequencerPatternRef ref;
    if (!valid_macro(value)) return ref;
    ref.active = true;
    ref.bank = static_cast<uint8_t>((value >> 4) & 0x0F);
    ref.pattern = static_cast<uint8_t>(value & 0x0F);
    return ref;
}

bool ArrangeViewModule::scene_is_empty(const songcore::SequencerArrangeScene& scene) {
    for (const auto& ref : scene.tracks)
        if (ref.active) return false;
    return true;
}

void ArrangeViewModule::draw(Canvas& c, int x, int y, const ArrangeViewState& s) const {
    const Theme& t = s.theme;
    c.fill_rect(x, y, WIDTH, HEIGHT, t.background);

    const int page = std::clamp(s.page, 0, SCENE_PAGE_COUNT - 1);
    const int cursorRow = std::clamp(s.cursorRow, 0, TRACK_COUNT - 1);
    const int cursorCol = std::clamp(s.cursorColumn, 0, SCENES_PER_PAGE - 1);

    c.draw_text("ARRANGE", x + LEFT, y + BANK_LABEL_Y, t.textTitle, CHAR_SPACING, FONT_SCALE);

    // The header gives the transport an explicit scene readout in addition to the inverted column
    // marker. This is especially useful when the current scene is off the visible page.
    if (s.playingScene >= 0) {
        const std::string sceneLabel = "SC" + songcore::hex2(s.playingScene);
        c.fill_rect(x + 420, y + 3, 52, 25, t.textValue);
        c.draw_text(sceneLabel, x + 425, y + 8, t.background, CHAR_SPACING, FONT_SCALE);
    }

    // Scene numbers are deliberately shown as a compact 0..F header.  The active cursor column
    // uses the same header marker as the other grid editors, while playback is a small triangle in
    // the gap before the currently playing scene.
    const int headerY = y + TOP - 17;
    for (int col = 0; col < SCENES_PER_PAGE; ++col) {
        const int scene = page * SCENES_PER_PAGE + col;
        const bool playing = scene == s.playingScene;
        const bool cursor = col == cursorCol;
        const int hx = x + matrix::cell_x(col);
        const std::string label = songcore::hex2(col).substr(1);
        const int labelW = Canvas::text_width(label, CHAR_SPACING, FONT_SCALE);
        const int labelX = hx + (matrix::OCCUPIED_SIZE - labelW) / 2;
        if (playing) {
            c.fill_rect(hx, headerY - 3, matrix::OCCUPIED_SIZE, 23, t.textValue);
            c.draw_text(label, labelX, headerY + 3, t.background, CHAR_SPACING, FONT_SCALE);
        } else {
            const Argb color = cursor ? cursor_mark_ink(t) : t.textParam;
            c.draw_text(label, labelX, headerY, color, CHAR_SPACING, FONT_SCALE);
        }
    }

    for (int track = 0; track < TRACK_COUNT; ++track) {
        const int rowY = y + matrix::cell_y(track);
        const Argb rowLabel = (track == cursorRow) ? cursor_mark_ink(t) : t.textParam;
        c.draw_text(std::to_string(track + 1), x, rowY + 8, rowLabel, CHAR_SPACING, FONT_SCALE);

        const songcore::SequencerArrangeScene* sceneData = nullptr;
        for (int col = 0; col < SCENES_PER_PAGE; ++col) {
            const int scene = page * SCENES_PER_PAGE + col;
            if (scene < static_cast<int>(s.sequencer.scenes.size()))
                sceneData = &s.sequencer.scenes[static_cast<size_t>(scene)];
            else
                sceneData = nullptr;

            songcore::SequencerPatternRef ref;
            if (sceneData) ref = sceneData->tracks[static_cast<size_t>(track)];

            const bool cursor = track == cursorRow && col == cursorCol;
            const bool playing = scene == s.playingScene;
            const int cellX = x + matrix::cell_x(col);
            const int cellY = rowY;

            // A cell is a complete macro reference. Empty cells stay as plain grey squares, matching
            // the mockup; populated cells are inverted so the two hex digits remain legible on the
            // handheld display.
            if (playing) {
                // Scene playhead is a column-level indicator: invert the active macro cell and
                // keep the header indicator above it. Empty cells get the same playhead marker.
                if (ref.active) {
                    c.fill_rect(cellX, cellY, matrix::OCCUPIED_SIZE, matrix::OCCUPIED_SIZE, 0xFFA0A0A0);
                    c.draw_text(macro_text(ref), cellX + 5, cellY + 10, 0xFF000000,
                                CHAR_SPACING, FONT_SCALE);
                } else {
                    c.fill_rect(cellX + matrix::empty_offset(), cellY + matrix::empty_offset(), matrix::EMPTY_SIZE, matrix::EMPTY_SIZE, t.textPlayhead);
                }
            } else if (cursor) {
                if (ref.active) {
                    c.fill_rect(cellX, cellY, matrix::OCCUPIED_SIZE, matrix::OCCUPIED_SIZE, 0xFFA0A0A0);
                    c.draw_text(macro_text(ref), cellX + 5, cellY + 10, 0xFF000000,
                                CHAR_SPACING, FONT_SCALE);
                } else {
                    c.fill_rect(cellX + matrix::empty_offset(), cellY + matrix::empty_offset(), matrix::EMPTY_SIZE, matrix::EMPTY_SIZE, matrix::EMPTY_COLOR);
                }
            } else if (ref.active) {
                c.fill_rect(cellX, cellY, matrix::OCCUPIED_SIZE, matrix::OCCUPIED_SIZE, 0xFFA0A0A0);
                c.draw_text(macro_text(ref), cellX + 5, cellY + 10, 0xFF000000,
                            CHAR_SPACING, FONT_SCALE);
            } else {
                c.fill_rect(cellX + matrix::empty_offset(), cellY + matrix::empty_offset(), matrix::EMPTY_SIZE, matrix::EMPTY_SIZE, matrix::EMPTY_COLOR);
            }
            if (cursor && !playing) matrix::draw_cursor(c, cellX, cellY, 0xFFFF0000);
        }
    }

    c.draw_text("ARRANGE", x + LEFT, y + FOOTER_Y, t.textTitle, CHAR_SPACING, FONT_SCALE);
    // Page selector 0..7.  The selector is a page, not a pattern bank: it chooses which sixteen
    // scene columns are visible and therefore keeps the scene address independent of track/bank.
    for (int p = 0; p < SCENE_PAGE_COUNT; ++p) {
        const int px = x + 197 + p * 24;
        if (p == page) {
            c.fill_rect(px - 2, y + FOOTER_Y - 4, 20, 27, 0xFFFFFFFF);
            c.draw_text(songcore::hex2(p).substr(1), px + 3, y + FOOTER_Y + 2,
                        0xFF000000, CHAR_SPACING, FONT_SCALE);
            if (s.pageSelector) {
                c.stroke_rect(px - 5, y + FOOTER_Y - 7, 26, 33, 0xFFFF0000, 3);
            }
        } else {
            const bool filledPage = [&] {
                const int first = p * SCENES_PER_PAGE;
                const int last = first + SCENES_PER_PAGE;
                for (int scene = first; scene < last && scene < static_cast<int>(s.sequencer.scenes.size()); ++scene)
                    if (!scene_is_empty(s.sequencer.scenes[static_cast<size_t>(scene)])) return true;
                return false;
            }();
            c.draw_text(songcore::hex2(p).substr(1), px + 3, y + FOOTER_Y + 2,
                        filledPage ? t.textValue : t.textEmpty, CHAR_SPACING, FONT_SCALE);
        }
    }
}

CursorContext ArrangeViewModule::cursor_context(const ArrangeViewState& s) const {
    const int scene = absolute_scene(s.page, s.cursorColumn);
    if (!valid_track(s.cursorRow) || scene < 0) return cc::none();

    const int current = (scene < static_cast<int>(s.sequencer.scenes.size()))
        ? macro_value(s.sequencer.scenes[static_cast<size_t>(scene)].tracks[static_cast<size_t>(s.cursorRow)])
        : -1;
    return cc::hex_byte(current == -1 ? 0 : current, 0, 255, -1,
                        /*can_delete=*/true, /*can_insert=*/true, /*can_create=*/true);
}

ArrangeInputResult ArrangeViewModule::handle_input(songcore::SequencerData& data, int page,
                                                    int cursor_row, int cursor_column,
                                                    const InputAction& action) const {
    ArrangeInputResult result;
    const int scene = absolute_scene(page, cursor_column);
    if (!valid_track(cursor_row) || scene < 0) return result;

    const int before = (scene < static_cast<int>(data.scenes.size()))
        ? macro_value(data.scenes[static_cast<size_t>(scene)].tracks[static_cast<size_t>(cursor_row)])
        : -1;

    switch (action.type) {
        case ActionType::SET_VALUE: {
            if (!valid_macro(action.value)) break;
            ensure_scene(data, scene);
            auto& ref = data.scenes[static_cast<size_t>(scene)].tracks[static_cast<size_t>(cursor_row)];
            ref = ref_from_macro(action.value);
            result.hasPattern = true;
            result.lastEditedBank = ref.bank;
            result.lastEditedPattern = ref.pattern;
            break;
        }
        case ActionType::INSERT_DEFAULT:
            ensure_scene(data, scene);
            data.scenes[static_cast<size_t>(scene)].tracks[static_cast<size_t>(cursor_row)] =
                ref_from_macro(0);
            result.hasPattern = true;
            result.lastEditedBank = 0;
            result.lastEditedPattern = 0;
            break;
        case ActionType::DELETE:
            if (scene < static_cast<int>(data.scenes.size()))
                data.scenes[static_cast<size_t>(scene)].tracks[static_cast<size_t>(cursor_row)] = {};
            break;
        default:
            break;
    }

    const int after = (scene < static_cast<int>(data.scenes.size()))
        ? macro_value(data.scenes[static_cast<size_t>(scene)].tracks[static_cast<size_t>(cursor_row)])
        : -1;
    result.modified = before != after;

    // Trim trailing empty scenes. This keeps persistence sparse and means that writing and then
    // deleting the last arrangement cell returns the project to the same representation as before.
    while (!data.scenes.empty() && scene_is_empty(data.scenes.back())) data.scenes.pop_back();
    return result;
}

} // namespace pt::ui
