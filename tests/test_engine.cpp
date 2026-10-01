#include "project/Project.h"
#include "sequencer/Sequencer.h"
#include "sequencer/TrackPlayer.h"
#include <cassert>
#include <iostream>
#include <memory>

using namespace ss;

static void testPatternLength() {
    Pattern p; p.setLength(13); assert(p.length == 13);
}
static void testPolymeter() {
    Pattern a, b; a.setLength(16); b.setLength(13);
    TrackPlayer pa, pb; pa.setPattern(&a); pb.setPattern(&b);
    for (int i = 0; i < 100; ++i) {
        assert(pa.stepAtTransport(i * PPQ) < 16);
        assert(pb.stepAtTransport(i * PPQ) < 13);
    }
}
static void testDirections() {
    Pattern p; p.setLength(4); TrackPlayer player; player.setPattern(&p);
    assert(player.stepAtTransport(0) == 0);
    assert(player.stepAtTransport(PPQ) == 1);
    p.direction = PlayDirection::Reverse;
    assert(player.stepAtTransport(0) == 3);
    assert(player.stepAtTransport(PPQ) == 2);
    p.direction = PlayDirection::PingPong;
    assert(player.stepAtTransport(0) == 0);
    assert(player.stepAtTransport(PPQ) == 1);
    assert(player.stepAtTransport(2 * PPQ) == 2);
    assert(player.stepAtTransport(3 * PPQ) == 3);
    assert(player.stepAtTransport(4 * PPQ) == 3);
    assert(player.stepAtTransport(5 * PPQ) == 2);
    assert(player.stepAtTransport(6 * PPQ) == 1);
    assert(player.stepAtTransport(7 * PPQ) == 0);
    assert(player.stepAtTransport(8 * PPQ) == 0);
}
static void testPatternSpeed() {
    Pattern p; p.setLength(4);
    TrackPlayer player; player.setPattern(&p);

    p.speed = SpeedMultiplier::Half;
    assert(player.stepAtTransport(0) == 0);
    assert(player.stepAtTransport(PPQ) == 0);
    assert(player.stepAtTransport(2 * PPQ) == 1);
    assert(player.stepAtTransport(3 * PPQ) == 1);

    p.speed = SpeedMultiplier::One;
    assert(player.stepAtTransport(PPQ) == 1);
    assert(player.stepAtTransport(2 * PPQ) == 2);

    p.speed = SpeedMultiplier::Double;
    assert(player.stepAtTransport(PPQ) == 2);
    assert(player.stepAtTransport(2 * PPQ) == 0);

    p.speed = SpeedMultiplier::Quadruple;
    assert(player.stepAtTransport(PPQ) == 0);
    assert(player.stepAtTransport(2 * PPQ) == 0);
    assert(player.stepAtTransport(3 * PPQ) == 0);
    assert(player.stepAtTransport(4 * PPQ) == 0);
    assert(player.stepAtTransport(5 * PPQ) == 1);
}
static void testAllParametersAndDefaults() {
    Instrument inst; inst.defaults[static_cast<size_t>(Parameter::Volume)] = ParameterValue::Set(0xEF);
    Step s; assert(!s.get(Parameter::Volume).set);
    assert(inst.resolve(Parameter::Volume, s.get(Parameter::Volume)).value == 0xEF);
    s.set(Parameter::Volume, ParameterValue::Set(0x40));
    assert(inst.resolve(Parameter::Volume, s.get(Parameter::Volume)).value == 0x40);
    s.set(Parameter::Volume, ParameterValue::Unset());
    assert(inst.resolve(Parameter::Volume, s.get(Parameter::Volume)).value == 0xEF);
}
static void testLaunchModes() {
    auto project = std::make_unique<Project>(); Sequencer seq(*project); seq.transport().setPosition(PPQ + 3);
    seq.launcher(0).request(project->tracks[0], 2, 3, LaunchMode::Quantized, seq.transport());
    assert(project->tracks[0].activeBank == 0);
    seq.transport().setPosition(seq.transport().nextDownbeat());
    seq.launcher(0).applyIfDue(project->tracks[0], seq.transport());
    assert(project->tracks[0].activeBank == 2 && project->tracks[0].activePattern == 3);

    seq.transport().setPosition(1234);
    seq.launcher(0).request(project->tracks[0], 1, 7, LaunchMode::Legato, seq.transport());
    seq.transport().setPosition(1234);
    seq.launcher(0).applyIfDue(project->tracks[0], seq.transport());
    assert(project->tracks[0].activeBank == 2);
    seq.transport().setPosition(1235);
    seq.launcher(0).applyIfDue(project->tracks[0], seq.transport());
    assert(project->tracks[0].activeBank == 1 && project->tracks[0].activePattern == 7);
}
static void testSequencer() {
    auto project = std::make_unique<Project>();
    auto& p = project->tracks[0].active();
    p.setLength(13);
    p.step(0).set(Parameter::Note, ParameterValue::Set(60));
    p.step(1).set(Parameter::Note, ParameterValue::Set(62));
    Sequencer seq(*project);
    auto e = seq.eventsAt(0); assert(e.size() == 1 && e[0].note == 60);
    e = seq.eventsAt(PPQ); assert(e.size() == 1 && e[0].note == 62);
}
int main() {
    testPatternLength(); testPolymeter(); testDirections(); testPatternSpeed(); testAllParametersAndDefaults(); testLaunchModes(); testSequencer();
    std::cout << "SpriteStep engine tests: PASS\n";
    return 0;
}
