#include "banks.h"

#include <cassert>
#include <iostream>

int main() {
    sequencer::Project project;
    // C major: C D E F G A B.
    constexpr unsigned C_MAJOR = (1u << 0) | (1u << 2) | (1u << 4) | (1u << 5) |
                                  (1u << 7) | (1u << 9) | (1u << 11);

    sequencer::BanksController banks(project);
    constexpr int TRACK_INSTRUMENT = 7;
    banks.randomize_track_pattern(0, 0, 0, 0x12345678u, C_MAJOR, 0, TRACK_INSTRUMENT);

    bool sawEmpty = false;
    bool sawNote = false;
    for (const auto& step : project.tracks[0].banks[0].patterns[0].steps) {
        if (step.note == songcore::Note::EMPTY()) {
            sawEmpty = true;
            assert(step.instrument == 0);
            assert(step.volume == 0x7F);
            continue;
        }
        sawNote = true;
        assert(step.instrument == TRACK_INSTRUMENT);
        const int midi = songcore::note_to_midi(step.note);
        const int degree = ((midi % 12) + 12) % 12;
        assert((C_MAJOR & (1u << degree)) != 0);
    }
    assert(sawEmpty);
    assert(sawNote);

    std::cout << "banks randomization tests: PASS\n";
    return 0;
}
