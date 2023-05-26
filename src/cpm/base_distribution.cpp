#include <fstream>
#include <algorithm>
#include <list>
#include "base_distribution.hpp"

void UniformDistribution::setBaseDistribution(std::unordered_map<wchar_t, int> histogram) {
    distribution.clear();

    for (auto& pair : histogram)
        distribution[pair.first] = 1.0 / histogram.size();
}

std::unordered_map<wchar_t, double> UniformDistribution::getDistributionWithContext(std::wstring_view context) {
    return distribution;
}

void FrequencyDistribution::setBaseDistribution(std::unordered_map<wchar_t, int> histogram){
    distribution.clear();

    int total = 0;
    for (auto& pair : histogram)
        total += pair.second;

    for (auto& pair : histogram)
        distribution[pair.first] = (double) pair.second / total;
}

std::unordered_map<wchar_t, double> FrequencyDistribution::getDistributionWithContext(std::wstring_view context) {
    return distribution;
}

void FiniteContextDistribution::setBaseDistribution(std::unordered_map<wchar_t, int> histogram) {
    // Initialize base distribution (it already contains the alphabet as the keys)
    distribution.clear();
    for (auto& pair : histogram)
        distribution.insert({pair.first, 0});

    // Remove all symbols that don't belong to the alphabet from the counts in the context table
    for (auto& context_pair : context_table_counts) {
        auto& context_counts = context_pair.second;

        for (auto it = context_counts.begin(); it != context_counts.end();) {
            auto& pair = *it; 
            if (distribution.find(pair.first) == distribution.end())
                it = context_counts.erase(it);
            else
                it++;
        }
    }
}

void FiniteContextDistribution::updateWithContext(std::wstring_view context, wchar_t symbol) {
    std::wstring_view sub_context(context.data() + context.size() - k, k);
    
    context_table_counts[sub_context].insert({symbol, 0});
    context_table_counts[sub_context][symbol]++;
}

std::unordered_map<wchar_t, double> FiniteContextDistribution::getDistributionWithContext(std::wstring_view context) {
    std::wstring sub_context(context.data() + context.size() - k, k);
    
    // Get the sum of counts of the letters seen after sub-context.
    // If the sub-context was never seen when building the context table, then we will simply build the uniform distribution
    bool sub_context_has_counts = context_table_counts.find(sub_context) != context_table_counts.end();
    int sum = 0;
    if (sub_context_has_counts)
        for (auto& pair : context_table_counts.at(sub_context))
            sum += pair.second;

    // Reset the probability distribution, setting all values to the default probability (as if their count is 0).
    // The size of the alphabet is the same as the size of the distribution, since its keys compose the alphabet
    for (auto& pair : distribution)
        pair.second = alpha / (sum + alpha * distribution.size());

    // Update the probabilities for the characters that did end up having counts
    if (sub_context_has_counts)
        for (auto& pair : context_table_counts.at(sub_context)) {
            unsigned int& count = pair.second;
            distribution.at(pair.first) = (pair.second + alpha) / (sum + alpha * distribution.size());
        }
    
    return distribution;
}
