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
    auto &qpat = q.sequencer.tracks[0].banks[3].patterns[5];
    assert(qpat.length == 7);
    assert(qpat.step_duration_multiplier == 2);
    assert(qpat.direction == SequencerDirection::REVERSE);
    assert(qpat.shuffle == 200);
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
    assert(legacyProject.sequencer.tracks[0].banks[0].patterns[0].length == 16);

    nlohmann::json legacySeq = {
        {"version", 2},
        {"tracks", nlohmann::json::array({nlohmann::json{
            {"stepDurationMultiplier", 4}, {"direction", static_cast<int>(SequencerDirection::PINGPONG)},
            {"shuffle", 123}, {"patterns", nlohmann::json::array({nlohmann::json{
                {"bank", 0}, {"pattern", 0}, {"length", 4}, {"steps", nlohmann::json::array()}
            }})}
        }})}
    };
    auto migrated = detail::parse_sequencer(legacySeq);
    const auto& migratedPattern = migrated.tracks[0].banks[0].patterns[0];
    assert(migratedPattern.step_duration_multiplier == 4);
    assert(migratedPattern.direction == SequencerDirection::PINGPONG);
    assert(migratedPattern.shuffle == 123);

    std::cout << "sequencer project IO tests: PASS\n";
}
