#include <iostream>
#include <fstream>
#include <algorithm>
#include <list>
#include <cmath>
#include <locale>
#include <codecvt>
#include "cpm.hpp"

void CopyModel::initializeWithMostFrequent() {
    auto max_pair = std::max_element(alphabet_counts.begin(), alphabet_counts.end(),
            [](const std::pair<wchar_t, int>& x, const std::pair<wchar_t, int>& y) {return x.second < y.second;}
    );

    current_pattern = std::wstring(k, max_pair->first);
    copy_pattern = current_pattern;
}

// Return true if the pattern is already in the map
bool CopyModel::registerPattern() {
    return pointer_manager->registerCopyPointer(current_pattern, current_position);
}

bool CopyModel::isPatternRegistered() {
    return pointer_manager->isPatternRegistered(current_pattern);
}

// TODO: PERFORMANCE: circular array for current_pattern
// TODO: PERFORMANCE: avoid changing the current_pattern when passing through the reference text, we can simply increment current_position
void CopyModel::advance() {
    // Update the base distribution with the current context (the current pattern) before advancing
    base_distribution->updateWithContext(current_pattern, mem_file.at(current_position + 1));
    // Update current pattern and advance read pointer (current_position)
    current_pattern += mem_file.at(++current_position);
    current_pattern.erase(0, 1);
    // Advance copy pointer
    copy_position++;
}

// Returns true when able to predict
bool CopyModel::predictionSetup(bool pattern_has_past) {

    hit_probability = calculateProbability(
        pointer_manager->getHits(current_pattern),
        pointer_manager->getMisses(current_pattern));

    bool can_predict = copy_position != current_position;

    if (!can_predict && pattern_has_past) {
        // The copy_pointer_index is initialized at 0. Therefore, the copy_position should be the only other pointer that doesn't point to the current position
        copy_position = pointer_manager->getCopyPointer(current_pattern);
        copy_pattern = current_pattern;
    }
    // Check whether copy pointer should be changed
    else if (surpassedAnyThreshold(hit_probability)) {

        pointer_manager->repositionCopyPointer(copy_pattern, &mem_file);
        // Change copy pointer to a new one, this one being from the current pattern
        copy_pattern = current_pattern;
        // If the model is learning (registering patterns), then we can attempt another copy right now (since current_pattern must have been registered)
        if (pattern_has_past)
            copy_position = pointer_manager->getCopyPointer(current_pattern);
        // Otherwise, we have to guess from now on, instead of starting another copy
        else
            copy_position = current_position;

        pointer_manager->reset();
        for (int i = 0; i < pointer_threshold_number; i++)
            pointer_threshold[i]->reset();
    }

    return copy_position != current_position;
}

bool CopyModel::predict() {

    prediction = mem_file.at(copy_position + 1);
    actual = mem_file.at(current_position + 1);

    bool hit = prediction == actual;

    pointer_manager->reportPrediction(current_pattern, hit);

    // Update internal probability distribution
    setRemainderProbabilities(prediction, 1.0 - hit_probability, base_distribution->getDistributionWithContext(current_pattern));
    // TODO: raise error if not present?
    probability_distribution[prediction] = hit_probability;

    return hit;
}

void CopyModel::firstPass(std::string file_name) {
    
    std::wifstream file(file_name);
    file.imbue(std::locale(file.getloc(), new std::codecvt_utf8<wchar_t>));

    wchar_t c = file.get();
    
    while (!file.eof()) {
        mem_file.push_back(c);

        alphabet_counts.insert({c, 0});
        alphabet_counts[c]++;

        c = file.get();
    }

    file.close();

    base_distribution->setBaseDistribution(alphabet_counts);
    for (auto pair : alphabet_counts)
        probability_distribution[pair.first] = 0;
}

void CopyModel::appendFuture(std::string file_name) {
    
    std::wifstream file(file_name);
    file.imbue(std::locale(file.getloc(), new std::codecvt_utf8<wchar_t>));

    wchar_t c = file.get();
    
    while (!file.eof()) {
        mem_file.push_back(c);
        c = file.get();
    }

    file.close();
}

bool CopyModel::eof() {
    // We add one because we don't want to predict a character outside of the stream, so we end earlier
    return current_position + 1 >= mem_file.size();
}

int CopyModel::countOf(wchar_t c) {
    return alphabet_counts[c];
}

double CopyModel::calculateProbability(int hits, int misses) {
    return (hits + alpha) / (hits + misses + 2 * alpha);
}

void CopyModel::setRemainderProbabilities(wchar_t exception, double probability_to_distribute, std::map<wchar_t, double> distribution) {
    double base_remainder_total = 0.0;
    for (auto pair : distribution)
        if (pair.first != exception)
            base_remainder_total += pair.second;
    
    for (auto pair : distribution)
        if (pair.first != exception)
            probability_distribution[pair.first] = probability_to_distribute * distribution[pair.first] / base_remainder_total;
}

double CopyModel::progress() {
    return (double) current_position / mem_file.size();
}

void CopyModel::guess() {

    actual = mem_file.at(current_position + 1);
    
    // Just return the base distribution
    prediction = '\0';
    hit_probability = 0;
    probability_distribution = base_distribution->getDistributionWithContext(current_pattern);
}

bool CopyModel::surpassedAnyThreshold(double hit_probability) {
    bool res = false;
    for (int i = 0; i < pointer_threshold_number; i++)
        res = res or pointer_threshold[i]->surpassedThreshold(hit_probability);
    return res;
}