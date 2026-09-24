#pragma once

#include <array>
#include <functional>
#include <string>

#include "sequencer/sequencer_model.h"
#include "ui/canvas.h"
#include "ui/cursor.h"
#include "ui/playhead.h"
#include "ui/theme.h"

namespace pt::ui {

enum class PatternParameter {
    NOTE,
    INSTRUMENT,
    VOLUME,
    PAN,
    SLIDE,
    CHANCE,
    ARPEGGIATOR,
    FILTER_FREQUENCY,
    RESONANCE,
    DRIVE,
    CRUSH,
    DOWNSAMPLE,
    REVERSE,
    REVERB,
    DELAY,
    // Kept for compatibility with the underlying pattern metadata/editor plumbing.
    CONDITION,
    WAIT,
    TRIGLESS,
    MORE
};

struct PatternEditorState {
    std::array<const sequencer::Pattern*, sequencer::TRACK_COUNT> patterns{};
    std::array<int, sequencer::TRACK_COUNT> banks{};
    std::array<int, sequencer::TRACK_COUNT> patternIndices{};
    std::array<int, sequencer::TRACK_COUNT> stepDurationMultipliers{};
    int track = 0;
    int bank = 0;
    int patternIndex = 0;
    int cursorStep = 0;
    PatternParameter parameter = PatternParameter::NOTE;
    TrackPlayhead playhead{}; // selected-track compatibility/readout
    Theme theme = theme_classic();
    bool isPlaying = false;
    std::array<TrackPlayhead, sequencer::TRACK_COUNT> playheads{};
    bool rateLengthHighlight = false;
    // 0 = grid, 1 = direction icon, 2 = shuffle icon. This is a small header focus, reached
    // by D-pad UP from Track 1 and returned from with D-pad DOWN.
    int headerControl = 0;
    int direction = 0;
    int shuffle = 0;
    unsigned scaleMask = 0x0FFFu;
    int scaleKey = 0;
};

struct PatternEditResult {
    bool modified = false;
    bool copied = false;
    bool cut = false;
    bool pasted = false;
};

class PatternEditorModule {
public:
    static constexpr int WIDTH = 610;
    static constexpr int HEIGHT = 390;
    static constexpr int STEP_COUNT = sequencer::MAX_PATTERN_STEPS;

    void draw(Canvas& c, int x, int y, const PatternEditorState& s) const;
    CursorContext cursor_context(const PatternEditorState& s) const;
    PatternEditResult handle_input(sequencer::Pattern& pattern, PatternEditorState& state,
                                   const InputAction& action) const;

    static bool step_has_data(const sequencer::PatternStep& step);
    static bool parameter_present(const sequencer::PatternStep& step, PatternParameter p);
    static int parameter_value(const sequencer::PatternStep& step, PatternParameter p);
    static int parameter_value(const sequencer::Pattern& pattern, int stepIndex, PatternParameter p);
    static void set_parameter(sequencer::PatternStep& step, PatternParameter p, int value);
    static void set_parameter(sequencer::Pattern& pattern, int stepIndex, PatternParameter p, int value);
    static void clear_parameter(sequencer::PatternStep& step, PatternParameter p);
    static void clear_parameter(sequencer::Pattern& pattern, int stepIndex, PatternParameter p);
    static std::string parameter_text(const sequencer::PatternStep& step, PatternParameter p);
    static PatternParameter footer_parameter(int index);
    static constexpr int FOOTER_PARAMETER_COUNT = 8;

private:
    static const char* parameter_label(PatternParameter p);
    static const char* parameter_name(PatternParameter p);
    static int fx_code(PatternParameter p);
    static int fx_slot(const sequencer::PatternStep& step, int code);
    static int ensure_fx_slot(sequencer::PatternStep& step, int code);
    static bool parameter_editable(PatternParameter p);
};

} // namespace pt::ui
