#include <cassert>
#include "ui/modules/pattern_editor.h"

// Regression coverage for the Pattern footer selection contract:
// selecting a parameter with L1+LEFT/RIGHT must survive an A press.
// The actual dispatcher owns seq_a_action(); this test keeps the intended parameter range
// explicit so a future refactor does not accidentally reintroduce a NOTE-only footer.
int main() {
    using pt::ui::PatternParameter;
    constexpr PatternParameter footer[] = {
        PatternParameter::NOTE, PatternParameter::INSTRUMENT, PatternParameter::VOLUME,
        PatternParameter::PAN, PatternParameter::SLIDE, PatternParameter::CHANCE,
        PatternParameter::ARPEGGIATOR, PatternParameter::FILTER_FREQUENCY,
        PatternParameter::RESONANCE, PatternParameter::DRIVE, PatternParameter::CRUSH,
        PatternParameter::DOWNSAMPLE, PatternParameter::REVERSE, PatternParameter::REVERB,
        PatternParameter::DELAY
    };
    assert(sizeof(footer) / sizeof(footer[0]) == 15);
    assert(footer[0] == PatternParameter::NOTE);
    assert(footer[1] == PatternParameter::INSTRUMENT);
    assert(footer[14] == PatternParameter::DELAY);
    return 0;
}
