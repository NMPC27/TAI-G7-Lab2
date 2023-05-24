#include <fstream>
#include <algorithm>
#include <list>
#include "base_distribution.hpp"

void UniformDistribution::setBaseDistribution(std::unordered_map<wchar_t, int> histogram) {
    distribution.clear();

    for (auto& pair : histogram)
        distribution[pair.first] = 1.0 / histogram.size();
}

std::map<wchar_t, double> UniformDistribution::getDistributionWithContext(std::wstring_view context) {
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

std::map<wchar_t, double> FrequencyDistribution::getDistributionWithContext(std::wstring_view context) {
    return distribution;
}

void FiniteContextDistribution::setBaseDistribution(std::unordered_map<wchar_t, int> histogram) {
    // Initialize alphabet and base distribution
    alphabet.clear();
    distribution.clear();
    for (auto& pair : histogram) {
        alphabet.push_back(pair.first);
        distribution.insert({pair.first, 0});
    }
    std::sort(alphabet.begin(), alphabet.end());    // sorting is very important, because the probability distribution is an ordered map 
                                                    // and will make use of this assumption when being built

    // Synchronize finite context model with the target alphabet
    for (auto& context_counts_pair : context_table_counts) {
        auto& context = context_counts_pair.first;
        auto& context_counts = context_counts_pair.second;

        // Add all characters that were in the target
        context_table_distributions.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(context),
            std::forward_as_tuple(alphabet.size(), 0.0));   // can use count at 0 since probabilities will be smoothed
        auto& probabilities_vector = context_table_distributions.at(context);

        for (auto& count_pair : context_counts) {
            auto& symbol = count_pair.first;
            auto& count = count_pair.second;

            auto symbol_it = std::lower_bound(alphabet.begin(), alphabet.end(), symbol);
            
            // If this symbol is in the histogram, then we add the counts trained on the reference
            if ((symbol_it != alphabet.end()) && (*symbol_it == symbol))
                context_table_distributions.at(context).at(symbol_it - alphabet.begin()) = count;
        }

        int sum = 0;
        for (auto& count : probabilities_vector)
            sum += count;

        // Convert the counts to probabilities
        for (auto& count : probabilities_vector)
            count = (count + alpha) / (sum + alpha * histogram.size());
    }

    context_table_counts.clear();   // not needed anymore
}

void FiniteContextDistribution::updateWithContext(std::wstring_view context, wchar_t symbol) {
    std::wstring_view sub_context(context.data() + context.size() - k, k);
    
    context_table_counts[sub_context].insert({symbol, 0});
    context_table_counts[sub_context][symbol]++;
}

std::map<wchar_t, double> FiniteContextDistribution::getDistributionWithContext(std::wstring_view context) {
    std::wstring sub_context(context.data() + context.size() - k, k);
    // In case the pattern was never seen before, just treat it as an uniform distribution (the result of the formula applied in setBaseDistribution with 0 counts)
    context_table_distributions.try_emplace(sub_context, alphabet.size(), 1.0 / alphabet.size());
    // Update the distribution's values with the context's
    int i = 0;
    for (auto& pair : distribution) {
        pair.second = context_table_distributions.at(sub_context).at(i);
        i++;
    }
    return distribution;
}
