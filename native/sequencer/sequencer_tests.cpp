#include "sequencer.h"

#include <cassert>
#include <iostream>

using namespace sequencer;

static Pattern& make_pattern(Project& p, int track, int bank, int pattern, int length) {
    Pattern& result = pattern_at(p, track, bank, pattern);
    result.length = static_cast<uint8_t>(length);
    return result;
}

static void test_cycle_lengths() {
    Project p;
    make_pattern(p, 0, 0, 0, 16);
    make_pattern(p, 1, 0, 0, 16);
    p.tracks[0].step_duration_multiplier = 1;
    p.tracks[1].step_duration_multiplier = 2;

    assert(pattern_duration_base_steps(pattern_at(p, 0, 0, 0), p.tracks[0]) == 16);
    assert(pattern_duration_base_steps(pattern_at(p, 1, 0, 0), p.tracks[1]) == 32);
}

static void test_shared_clock_different_rates() {
    Project p;
    ArrangeScene s;
    s.tracks[0] = PatternRef{true, 0, 0};
    s.tracks[1] = PatternRef{true, 0, 0};
    p.scenes.push_back(s);
    make_pattern(p, 0, 0, 0, 16);
    make_pattern(p, 1, 0, 0, 16);
    p.tracks[0].step_duration_multiplier = 1;
    p.tracks[1].step_duration_multiplier = 2;

    Sequencer seq(p, 48000);
    seq.set_tempo(120);
    seq.start_arrange(0, 0);

    const int64_t base = seq.base_step_frames();
    auto events = seq.schedule_until(base * 16);

    int lastT1 = -1;
    int lastT2 = -1;
    for (const auto& e : events) {
        if (e.track == 0 && e.frame < base * 16) lastT1 = e.step;
        if (e.track == 1 && e.frame < base * 16) lastT2 = e.step;
    }

    // At the 16th-note boundary, track 1 has completed one 16-step cycle while track 2 has reached
    // step 8 (zero-based step 7 was the last event before the boundary).
    assert(lastT1 == 15);
    assert(lastT2 == 7);
}

static void test_short_pattern() {
    Project p;
    ArrangeScene s;
    s.tracks[0] = PatternRef{true, 0, 0};
    p.scenes.push_back(s);
    make_pattern(p, 0, 0, 0, 4);
    p.tracks[0].step_duration_multiplier = 3;

    assert(pattern_duration_base_steps(pattern_at(p, 0, 0, 0), p.tracks[0]) == 12);
}

static void test_scene_duration_is_longest_effective_pattern() {
    Project p;
    ArrangeScene s;
    s.tracks[0] = PatternRef{true, 0, 0};
    s.tracks[1] = PatternRef{true, 0, 0};
    p.scenes.push_back(s);
    make_pattern(p, 0, 0, 0, 16);
    make_pattern(p, 1, 0, 0, 16);
    p.tracks[0].step_duration_multiplier = 1;
    p.tracks[1].step_duration_multiplier = 2;

    Sequencer seq(p, 48000);
    seq.start_arrange();
    assert(seq.runtime().scene_end_frame == seq.base_step_frames() * 32);
}

static void test_cue_waits_for_pattern_boundary() {
    Project p;
    ArrangeScene s;
    s.tracks[0] = PatternRef{true, 0, 0};
    s.tracks[1] = PatternRef{true, 0, 0};
    p.scenes.push_back(s);
    make_pattern(p, 0, 0, 0, 4);
    make_pattern(p, 0, 0, 1, 4);
    make_pattern(p, 1, 0, 0, 8); // keeps the scene alive while track 0 changes pattern

    Sequencer seq(p, 48000);
    seq.start_arrange();
    seq.cue_pattern(0, 0, 1);

    const int64_t step = seq.base_step_frames();
    auto events = seq.schedule_until(step * 5);

    bool sawOld = false;
    bool sawNew = false;
    for (const auto& e : events) {
        if (e.track != 0) continue;
        if (e.pattern == 0) sawOld = true;
        if (e.pattern == 1) sawNew = true;
    }
    assert(sawOld);
    assert(sawNew);
}

static void test_same_pattern_continues_across_arrange_scenes() {
    Project p;
    ArrangeScene a;
    ArrangeScene b;
    a.tracks[0] = PatternRef{true, 0, 0};
    b.tracks[0] = PatternRef{true, 0, 0};
    p.scenes.push_back(a);
    p.scenes.push_back(b);
    make_pattern(p, 0, 0, 0, 4);

    Sequencer seq(p, 48000);
    seq.start_arrange(0, 0);
    const int64_t step = seq.base_step_frames();
    const int64_t sceneDuration = step * 4;
    auto first = seq.schedule_until(sceneDuration);
    assert(!first.empty());
    assert(seq.runtime().scene == 1);
    assert(seq.runtime().tracks[0].step == 0);
    assert(seq.runtime().tracks[0].pattern_repeat == 1);

    auto second = seq.schedule_until(sceneDuration * 2);
    bool sawStep0 = false;
    for (const auto& e : second) {
        if (e.track == 0 && e.frame == sceneDuration) sawStep0 = true;
    }
    assert(sawStep0);
    assert(seq.runtime().tracks[0].pattern_repeat == 2);
}

static void test_condition_follows_pattern_repeat_across_scenes() {
    Project p;
    ArrangeScene a;
    a.tracks[0] = PatternRef{true, 0, 0};
    for (int i = 0; i < 4; ++i) p.scenes.push_back(a);
    auto& pat = make_pattern(p, 0, 0, 0, 2);
    pat.conditions[0] = 0x13; // 1 of 3

    Sequencer seq(p, 48000);
    seq.start_arrange(0, 0);
    const int64_t step = seq.base_step_frames();
    auto first = seq.schedule_until(step * 2);
    bool firstStep = false;
    for (const auto& e : first) if (e.step == 0) firstStep = true;
    assert(firstStep);
    auto second = seq.schedule_until(step * 4);
    bool secondStep = false;
    for (const auto& e : second) if (e.step == 0) secondStep = true;
    assert(!secondStep); // cycle 2 of 3
    auto third = seq.schedule_until(step * 6);
    bool thirdStep = false;
    for (const auto& e : third) if (e.step == 0) thirdStep = true;
    assert(!thirdStep); // cycle 3 of 3
    auto fourth = seq.schedule_until(step * 8);
    bool fourthStep = false;
    for (const auto& e : fourth) if (e.step == 0) fourthStep = true;
    assert(fourthStep); // cycle 4 -> next 1 of 3
}

static void test_direction_changes_authored_step_order_without_changing_cycle_duration() {
    Project p;
    ArrangeScene s;
    s.tracks[0] = PatternRef{true, 0, 0};
    p.scenes.push_back(s);
    auto& pat = make_pattern(p, 0, 0, 0, 4);
    p.tracks[0].direction = songcore::SequencerDirection::REVERSE;

    Sequencer seq(p, 48000);
    seq.start_arrange();
    const int64_t step = seq.base_step_frames();
    auto events = seq.schedule_until(step * 4);
    assert(events.size() == 4);
    assert(events[0].step == 3 && events[1].step == 2 && events[2].step == 1 && events[3].step == 0);
    assert(seq.runtime().scene_end_frame == step * 4);
}

static void test_shuffle_delays_every_other_step_but_keeps_grid_cycle() {
    Project p;
    ArrangeScene s;
    s.tracks[0] = PatternRef{true, 0, 0};
    p.scenes.push_back(s);
    make_pattern(p, 0, 0, 0, 4);
    p.tracks[0].shuffle = 255;

    Sequencer seq(p, 48000);
    seq.start_arrange();
    const int64_t step = seq.base_step_frames();
    auto events = seq.schedule_until(step * 4);
    assert(events.size() == 4);
    assert(events[0].frame == 0);
    assert(events[1].frame == step + step / 2);
    assert(events[2].frame == step * 2);
    assert(events[3].frame == step * 3 + step / 2);
    assert(seq.runtime().scene_end_frame == step * 4);
}

static void test_wait_delays_trigger_without_changing_grid() {
    Project p;
    ArrangeScene s;
    s.tracks[0] = PatternRef{true, 0, 0};
    p.scenes.push_back(s);
    auto& pat = make_pattern(p, 0, 0, 0, 2);
    pat.wait_ppqn[0] = 3; // FMS: half a step

    Sequencer seq(p, 48000);
    seq.start_arrange();
    const int64_t step = seq.base_step_frames();
    auto events = seq.schedule_until(step * 2);
    assert(events.size() == 2);
    assert(events[0].frame == step / 2);
    assert(events[0].wait_ppqn == 3);
    assert(events[0].duration_frames == step);
    assert(events[1].frame == step);
    assert(seq.runtime().scene_end_frame == step * 2);
}

static void test_trigless_preserves_grid_and_marks_event() {
    Project p;
    ArrangeScene s;
    s.tracks[0] = PatternRef{true, 0, 0};
    p.scenes.push_back(s);
    auto& pat = make_pattern(p, 0, 0, 0, 2);
    pat.steps[0].note = songcore::Note::C4();
    pat.trigless[0] = 1;

    Sequencer seq(p, 48000);
    seq.start_arrange();
    const int64_t step = seq.base_step_frames();
    auto events = seq.schedule_until(step * 2);
    assert(events.size() == 2);
    assert(events[0].frame == 0);
    assert(events[0].trigless);
    assert(events[1].frame == step);
    assert(!events[1].trigless);
}

static void test_playhead_readback_uses_actual_event_frame() {
    Project p;
    ArrangeScene s;
    s.tracks[0] = PatternRef{true, 2, 5};
    p.scenes.push_back(s);
    auto& pat = make_pattern(p, 0, 2, 5, 2);
    pat.wait_ppqn[0] = 3;

    Sequencer seq(p, 48000);
    seq.start_arrange(0, 1000);
    const int64_t step = seq.base_step_frames();
    seq.schedule_until(1000 + step * 2 + 1);

    // At the nominal downbeat the first step has not fired yet because Wait=3 is half a step.
    assert(!seq.playhead_at(0, 1000));
    auto ph = seq.playhead_at(0, 1000 + step / 2);
    assert(ph);
    assert(ph->bank == 2 && ph->pattern == 5 && ph->step == 0);
    assert(ph->scene == 0);
    auto ph2 = seq.playhead_at(0, 1000 + step + 1);
    assert(ph2);
    assert(ph2->step == 1);
}

static void test_cue_readback_survives_lookahead_until_boundary() {
    Project p;
    ArrangeScene s;
    s.tracks[0] = PatternRef{true, 0, 0};
    s.tracks[1] = PatternRef{true, 0, 0};
    p.scenes.push_back(s);
    make_pattern(p, 0, 0, 0, 4);
    make_pattern(p, 0, 1, 2, 4);
    make_pattern(p, 1, 0, 0, 8);

    Sequencer seq(p, 48000);
    seq.start_arrange();
    seq.schedule_until(seq.base_step_frames() * 2);
    seq.cue_pattern(0, 1, 2);
    assert(seq.cue_state(0).pending);
    seq.schedule_until(seq.base_step_frames() * 4 + 1);
    assert(!seq.cue_state(0).pending);
    auto ph = seq.playhead_at(0, seq.base_step_frames() * 4);
    assert(ph && ph->bank == 1 && ph->pattern == 2);
}

int main() {
    test_cycle_lengths();
    test_shared_clock_different_rates();
    test_short_pattern();
    test_scene_duration_is_longest_effective_pattern();
    test_cue_waits_for_pattern_boundary();
    test_same_pattern_continues_across_arrange_scenes();
    test_condition_follows_pattern_repeat_across_scenes();
    test_direction_changes_authored_step_order_without_changing_cycle_duration();
    test_shuffle_delays_every_other_step_but_keeps_grid_cycle();
    test_wait_delays_trigger_without_changing_grid();
    test_trigless_preserves_grid_and_marks_event();
    test_playhead_readback_uses_actual_event_frame();
    test_cue_readback_survives_lookahead_until_boundary();
    std::cout << "sequencer tests: PASS\n";
    return 0;
}
