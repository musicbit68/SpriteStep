#pragma once
#include "core/Constants.h"
#include "core/Types.h"
#include "project/Pattern.h"
#include <array>

namespace ss {
struct Track {
    std::array<Pattern, MAX_PATTERNS_PER_TRACK> patterns{};
    BankIndex activeBank = 0;
    PatternIndex activePattern = 0;

    Pattern& pattern(BankIndex bank, PatternIndex index) {
        return patterns.at(static_cast<std::size_t>(bank) * PATTERNS_PER_BANK + index);
    }
    const Pattern& pattern(BankIndex bank, PatternIndex index) const {
        return patterns.at(static_cast<std::size_t>(bank) * PATTERNS_PER_BANK + index);
    }
    Pattern& active() { return pattern(activeBank, activePattern); }
    const Pattern& active() const { return pattern(activeBank, activePattern); }
};
}
