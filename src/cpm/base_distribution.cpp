#include <fstream>
#include <algorithm>
#include <list>
#include "base_distribution.hpp"

void UniformDistribution::setBaseDistribution(std::map<wchar_t, int> histogram) {
    distribution.clear();

    for (auto pair : histogram)
        distribution[pair.first] = 1.0 / histogram.size();
}

std::map<wchar_t, double> UniformDistribution::getDistributionWithContext(std::wstring context) {
    return distribution;
}

void FrequencyDistribution::setBaseDistribution(std::map<wchar_t, int> histogram){
    distribution.clear();

    int total = 0;
    for (auto pair : histogram)
        total += pair.second;

    for (auto pair : histogram)
        distribution[pair.first] = (double) pair.second / total;
}

std::map<wchar_t, double> FrequencyDistribution::getDistributionWithContext(std::wstring context) {
    return distribution;
}

void FiniteContextDistribution::setBaseDistribution(std::map<wchar_t, int> histogram) {
    alphabet.clear();
    for (auto pair : histogram)
        alphabet.push_back(pair.first);
}

void FiniteContextDistribution::updateWithContext(std::wstring context, wchar_t symbol) {
    std::wstring sub_context = context.substr(context.size() - k);

    // If the pattern was never seen before, initialize a new table
    if (context_table.find(sub_context) == context_table.end()) {
        
        context_table[sub_context] = std::map<wchar_t, double>();
        for (wchar_t alphabet_symbol : alphabet)
            context_table[sub_context][alphabet_symbol] = 0;
    }

    context_table[sub_context][symbol]++;
}

std::map<wchar_t, double> FiniteContextDistribution::getDistributionWithContext(std::wstring context) {
    std::wstring sub_context = context.substr(context.size() - k);
    std::map<wchar_t, double> distribution;
    
    int sum = 0;
    for (auto pair : context_table[context])
        sum += pair.second;

    for (auto pair : context_table[context])
        distribution[pair.first] = (context_table[context][pair.first] + alpha) / (sum + alpha * alphabet.size());

    return distribution;
}
