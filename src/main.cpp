#include "project/Project.h"
#include "sequencer/Sequencer.h"
#include <iostream>

int main(int argc, char**) {
    ss::Project project;
    auto& p = project.tracks[0].pattern(0, 0);
    p.setLength(16);
    p.step(0).set(ss::Parameter::Note, ss::ParameterValue::Set(60));
    p.step(0).set(ss::Parameter::Instrument, ss::ParameterValue::Set(0));
    p.step(8).set(ss::Parameter::Note, ss::ParameterValue::Set(67));

    ss::Sequencer seq(project);
    seq.transport().play();
    for (int i = 0; i < 32; ++i) {
        auto events = seq.eventsAt(seq.transport().position());
        for (const auto& e : events)
            std::cout << "tick=" << e.tick << " track=" << int(e.track)
                      << " note=" << int(e.note) << " inst=" << int(e.instrument) << '\n';
        seq.transport().advance(ss::PPQ);
    }
    return 0;
}
