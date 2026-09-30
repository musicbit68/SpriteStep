#pragma once
namespace ss {
constexpr int TRACK_COUNT = 8;
constexpr int BANK_COUNT = 8;
constexpr int PATTERNS_PER_BANK = 16;
constexpr int MAX_PATTERNS_PER_TRACK = BANK_COUNT * PATTERNS_PER_BANK;
constexpr int MAX_STEPS = 64;
constexpr int PPQ = 96;
}
