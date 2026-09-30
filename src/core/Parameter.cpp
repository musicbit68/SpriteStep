#include "core/Parameter.h"
namespace ss {
const char* parameterName(Parameter p) {
    switch (p) {
        case Parameter::Note: return "Note";
        case Parameter::Instrument: return "Instrument";
        case Parameter::Volume: return "Volume";
        case Parameter::Pan: return "Pan";
        case Parameter::Pitch: return "Pitch";
        case Parameter::Cutoff: return "Cutoff";
        case Parameter::Resonance: return "Resonance";
        case Parameter::Chance: return "Chance";
        case Parameter::Length: return "Length";
        case Parameter::Retrigger: return "Retrigger";
        case Parameter::Delay: return "Delay";
        default: return "Unknown";
    }
}
}
