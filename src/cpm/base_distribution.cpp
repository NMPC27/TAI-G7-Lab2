#include <fstream>
#include <algorithm>
#include <list>
#include "base_distribution.hpp"

void UniformDistribution::setBaseDistribution(std::map<wchar_t, int> histogram) {
    distribution.clear();

    for (auto& pair : histogram)
        distribution[pair.first] = 1.0 / histogram.size();
}

std::map<wchar_t, double> UniformDistribution::getDistributionWithContext(std::wstring_view context) {
    return distribution;
}

void FrequencyDistribution::setBaseDistribution(std::map<wchar_t, int> histogram){
    distribution.clear();

    int total = 0;
    for (auto& pair : histogram)
        total += pair.second;

    for (auto& pair : histogram)
        distribution[pair.first] = (double) pair.second / total;
}

std::map<wchar_t, double> FrequencyDistribution::getDistributionWithContext(std::wstring_view context) {
    return distribution;
}

void FiniteContextDistribution::setBaseDistribution(std::map<wchar_t, int> histogram) {
    // Synchronize finite context model with the target alphabet
    for (auto& context_pair : context_table) {
        auto& context_distribution = context_pair.second;

        // Remove characters that were in the reference but not in the target
        for (auto it = context_distribution.begin(); it != context_distribution.end();) {
            auto& pair = *it;
            if (histogram.find(pair.first) == histogram.end())
                it = context_distribution.erase(it);
            else
                it++;
        }

        // Add characters that were in the target but not in the reference
        for (auto& histogram_pair : histogram) {
            if (context_distribution.find(histogram_pair.first) == context_distribution.end())
                context_distribution[histogram_pair.first] = 0;    // can use count at 0 since probabilities will be smoothed
        }

        int sum = 0;
        for (auto& pair : context_distribution)
            sum += pair.second;

        for (auto& pair : context_distribution)
            context_distribution[pair.first] = (context_distribution[pair.first] + alpha) / (sum + alpha * histogram.size());
    }
    
    alphabet.clear();
    for (auto& pair : histogram)
        alphabet.push_back(pair.first);
}

void FiniteContextDistribution::updateWithContext(std::wstring_view context, wchar_t symbol) {
    std::wstring_view sub_context(context.data() + context.size() - k, k);

    // If the pattern was never seen before, initialize a new table
    if (context_table.find(sub_context) == context_table.end())
        context_table[sub_context][symbol] = 0;

    context_table[sub_context][symbol]++;
}

std::map<wchar_t, double> FiniteContextDistribution::getDistributionWithContext(std::wstring_view context) {
    std::wstring sub_context(context.data() + context.size() - k, k);
    if (context_table.find(sub_context) == context_table.end())
        for (auto symbol : alphabet)
            context_table[sub_context][symbol] = 1.0 / alphabet.size();
    return context_table[sub_context];
}
