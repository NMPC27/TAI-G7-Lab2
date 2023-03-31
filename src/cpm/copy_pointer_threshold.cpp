#include <fstream>
#include <algorithm>
#include <list>
#include "copy_pointer_threshold.hpp"

bool StaticCopyPointerThreshold::surpassedThreshold(double hit_probability) {
    return hit_probability < static_threshold;
}

void StaticCopyPointerThreshold::reset() {}

bool DerivativeCopyPointerThreshold::surpassedThreshold(double hit_probability) {
    if (previous_hit_probability == -1)
        previous_hit_probability = hit_probability;

    double derivative = hit_probability - previous_hit_probability;
    previous_hit_probability = hit_probability;
    return derivative < - derivative_threshold;
}

void DerivativeCopyPointerThreshold::reset() {
    previous_hit_probability = -1;
}

bool SuccessFailsCopyPointerThreshold::surpassedThreshold(double hit_probability) {
    if (previous_hit_probability == -1)
        previous_hit_probability = hit_probability;

    bool fail = hit_probability < previous_hit_probability;
    if (fail) fails_count++;
    else fails_count = fails_count > 0 ? fails_count - 1 : 0;

    return fails_count >= fails_threshold;
}

void SuccessFailsCopyPointerThreshold::reset() {
    previous_hit_probability = -1;
    fails_count = 0;
}