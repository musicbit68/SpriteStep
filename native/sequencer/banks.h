#ifndef SPRITESTEP_SEQUENCER_BANKS_H
#define SPRITESTEP_SEQUENCER_BANKS_H

#include <cstdint>
#include "sequencer_model.h"

namespace sequencer {

// Operations exposed by the BANKS screen. These are deliberately model-level operations so the
// same semantics can be used by the handheld UI, tests, and future keyboard/controller front ends.
class BanksController {
public:
    explicit BanksController(Project& project) : project_(project) {}

    void select_bank(int bank);
    void select_pattern(int pattern);
    void select_track(int track);

    int bank() const { return bank_; }
    int pattern() const { return pattern_; }
    int track() const { return track_; }

    Pattern& selected_pattern();
    const Pattern& selected_pattern() const;

    // X: clear values from one track's selected pattern. Pattern length is preserved.
    void clear_track_pattern(int track, int bank, int pattern);

    // ?: deterministic pseudo-randomization. The seed is optional so UI calls can use a changing
    // seed while tests can verify exact behavior. Pattern length is preserved.
    void randomize_track_pattern(int track, int bank, int pattern, uint32_t seed, unsigned scaleMask = 0x0FFFu, int scaleKey = 0, int instrumentId = 0);

    // A+B operations are just repeated track-local operations; keeping this here avoids the UI
    // having to duplicate the meaning of the ALL TRACKS modifier.
    void clear_all_selected_patterns();
    void randomize_all_selected_patterns(uint32_t seed, unsigned scaleMask = 0x0FFFu, int scaleKey = 0);

private:
    Project& project_;
    int track_ = 0;
    int bank_ = 0;
    int pattern_ = 0;
};

} // namespace sequencer

#endif
