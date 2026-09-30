#include "project/Pattern.h"
#include <algorithm>
#include <stdexcept>
namespace ss {
void Pattern::setLength(std::uint8_t n) {
    if (n < 1 || n > MAX_STEPS) throw std::out_of_range("Pattern length must be 1..64");
    length = n;
}
}
