#include "songcore/project_io.h"
#include <cassert>
#include <iostream>
int main() {
    using namespace songcore;
    Project p = make_default_project();
    p.version = 1;
    std::string legacy = serialize_project(p);
    nlohmann::json legacyJson = nlohmann::json::parse(legacy);
    assert(!legacyJson.contains("sequencer"));

    auto &pat = p.sequencer.tracks[0].banks[3].patterns[5];
    pat.step_duration_multiplier = 2;
    pat.direction = SequencerDirection::REVERSE;
    pat.shuffle = 200;
    pat.length = 7;
    pat.steps[0].note = Note::C4();
    pat.steps[0].instrument = 4;
    pat.steps[1].fx1Type = 0x16;
    pat.steps[1].fx1Value = 0x40;
    pat.conditions[2] = 0x13;
    pat.wait_ppqn[3] = 3;
    pat.wait_ppqn[4] = 6;
    pat.trigless[5] = 1;
    SequencerArrangeScene scene;
    scene.tracks[0].active = true;
    scene.tracks[0].bank = 3;
    scene.tracks[0].pattern = 5;
    p.sequencer.scenes.push_back(scene);

    std::string blob = serialize_project(p);
    auto j = nlohmann::json::parse(blob);
    assert(j.contains("sequencer"));
    Project q = parse_project(j);
    normalize_and_migrate(q);
    assert(q.sequencer.tracks[0].banks[3].patterns[5].step_duration_multiplier == 2);
    assert(q.sequencer.tracks[0].banks[3].patterns[5].direction == SequencerDirection::REVERSE);
    assert(q.sequencer.tracks[0].banks[3].patterns[5].shuffle == 200);
    auto &qpat = q.sequencer.tracks[0].banks[3].patterns[5];
    assert(qpat.length == 7);
    assert(qpat.steps[0].note == Note::C4());
    assert(qpat.steps[0].instrument == 4);
    assert(qpat.steps[1].fx1Type == 0x16 && qpat.steps[1].fx1Value == 0x40);
    assert(qpat.conditions[2] == 0x13);
    assert(qpat.wait_ppqn[3] == 3);
    assert(qpat.wait_ppqn[4] == 6);
    assert(qpat.trigless[5] == 1);
    assert(q.sequencer.scenes.size() == 1);
    assert(q.sequencer.scenes[0].tracks[0].active);
    assert(q.sequencer.scenes[0].tracks[0].bank == 3);
    assert(q.sequencer.scenes[0].tracks[0].pattern == 5);
    assert(serialize_project(q) == blob);

    // A legacy project with no sequencer object must load with an empty/default sequencer.
    Project legacyProject = parse_project(legacyJson);
    normalize_and_migrate(legacyProject);
    assert(legacyProject.sequencer.scenes.empty());
    assert(legacyProject.sequencer.tracks[0].step_duration_multiplier == 1);
    assert(legacyProject.sequencer.tracks[0].banks[0].patterns[0].length == 16);

    // Legacy track-level playback settings migrate into authored patterns.
    nlohmann::json legacySeq = { {"version", 2}, {"tracks", nlohmann::json::array()} };
    legacySeq["tracks"].push_back({{"stepDurationMultiplier", 4}, {"direction", static_cast<int>(SequencerDirection::PINGPONG)}, {"shuffle", 128}, {"patterns", nlohmann::json::array({{{"bank", 0}, {"pattern", 2}}})}});
    Project migrated = parse_project({{"version", 1}, {"sequencer", legacySeq}});
    normalize_and_migrate(migrated);
    const auto& mp = migrated.sequencer.tracks[0].banks[0].patterns[2];
    assert(mp.step_duration_multiplier == 4);
    assert(mp.direction == SequencerDirection::PINGPONG);
    assert(mp.shuffle == 128);

    std::cout << "sequencer project IO tests: PASS\n";
}
