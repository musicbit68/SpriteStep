#pragma once
#include "core/Types.h"
namespace ss {
enum class EventType { Note, Parameter, Transport };
struct Event { EventType type; Tick tick; TrackIndex track; };
}
