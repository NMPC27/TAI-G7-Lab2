#include <fstream>
#include <algorithm>
#include <list>
#include "base_distribution.hpp"

void UniformDistribution::setBaseDistribution(std::map<char, int> histogram) {
    distribution.clear();

    for (auto pair : histogram)
        distribution[pair.first] = 1.0 / histogram.size();
}

void FrequencyDistribution::setBaseDistribution(std::map<char, int> histogram){
    distribution.clear();

    int total = 0;
    for (auto pair : histogram)
        total += pair.second;

    for (auto pair : histogram)
        distribution[pair.first] = (double) pair.second / total;
}