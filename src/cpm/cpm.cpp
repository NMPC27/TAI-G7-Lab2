#include <iostream>
#include <fstream>
#include <algorithm>
#include <list>
#include <cmath>
#include <locale>
#include <codecvt>
#include "cpm.hpp"

void CopyModel::initializeOnReference() {
    current_pattern = std::wstring_view(reference_file.data(), k);
    copy_pattern = current_pattern;
    current_position = k-1;
    copy_position = k-1;
}

void CopyModel::initializeOnTarget() {
    current_pattern = std::wstring_view(target_file.data(), k);
    current_position = k-1;
    copy_position = k-1;
}

// Return true if the pattern is already in the map
bool CopyModel::registerPattern() {
    return pointer_manager->registerCopyPointer(current_pattern, current_position);
}

bool CopyModel::isPatternRegistered() {
    return pointer_manager->isPatternRegistered(current_pattern);
}

void CopyModel::updateDistribution() {
    // Update the base distribution with the current context (the current pattern) before advancing (should be training on the reference)
    base_distribution->updateWithContext(current_pattern, reference_file.at(current_position + 1));
}

void CopyModel::advance() {
    // Update current pattern and advance read pointer (current_position)
    current_pattern = std::wstring_view(current_pattern.data() + 1, current_pattern.size());
    current_position++;
    // Advance copy pointer, avoiding going out of the reference's bounds (assumes repeat-like wrapping, and so the end is repeated at the beginning)
    copy_position = (copy_position + 1) % (reference_file.size() - k);
}

// Returns true when able to predict
bool CopyModel::predictionSetup(bool pattern_has_past) {

    hit_probability = calculateProbability(
        pointer_manager->getHits(current_pattern),
        pointer_manager->getMisses(current_pattern));

    if (!predicting && pattern_has_past) {
        // The copy_pointer_index is initialized at 0. Therefore, the copy_position should be the only other pointer that doesn't point to the current position
        copy_position = pointer_manager->getCopyPointer(current_pattern);
        copy_pattern = current_pattern;
        predicting = true;
    }
    // Check whether copy pointer should be changed
    else if (surpassedAnyThreshold(hit_probability)) {

        pointer_manager->repositionCopyPointer(copy_pattern, &reference_file);
        // Change copy pointer to a new one, this one being from the current pattern
        copy_pattern = current_pattern;
        // If the model is learning (registering patterns), then we can attempt another copy right now (since current_pattern must have been registered)
        if (pattern_has_past) {
            copy_position = pointer_manager->getCopyPointer(current_pattern);
            predicting = true;
        }
        // Otherwise, we have to guess from now on, instead of starting another copy
        else {
            predicting = false;
        }

        pointer_manager->reset();
        for (int i = 0; i < pointer_threshold_number; i++)
            pointer_threshold[i]->reset();
    }

    // Can only predict if we predict a character that is actually in the target's alphabet
    return predicting && alphabet_counts.find(reference_file.at(copy_position + 1)) != alphabet_counts.end();
}

bool CopyModel::predict() {

    prediction = reference_file.at(copy_position + 1);
    actual = target_file.at(current_position + 1);

    bool hit = prediction == actual;

    pointer_manager->reportPrediction(current_pattern, hit);

    // Update internal probability distribution
    setRemainderProbabilities(prediction, 1.0 - hit_probability, base_distribution->getDistributionWithContext(current_pattern));
    probability_distribution.at(prediction) = hit_probability;

    return hit;
}

void CopyModel::firstPassOverReference(std::string reference_name) {
    reference_file.clear();

    std::wifstream file(reference_name);
    file.imbue(std::locale(file.getloc(), new std::codecvt_utf8<wchar_t>));

    // Reserve some space for the k-sized past (inserting at the beginning of the file in memory after reading everything would be painful)
    for (int i = 0; i < k; i++)
        reference_file.push_back(L'\0');

    wchar_t c = file.get();
    
    while (!file.eof()) {
        reference_file.push_back(c);

        alphabet_counts.insert({c, 0});
        alphabet_counts.at(c)++;

        c = file.get();
    }

    file.close();

    // Copy the last part of the file to the beginning, to serve as the past (repeat-like wrapping)
    for (int i = 0; i < k; i++)
        reference_file.at(i) = reference_file[reference_file.size() - k + i];
}

void CopyModel::firstPassOverTarget(std::string target_name) {
    target_file.clear();
    
    std::wifstream file(target_name);
    file.imbue(std::locale(file.getloc(), new std::codecvt_utf8<wchar_t>));

    // Reserve some space for the k-sized past (inserting at the beginning of the file in memory after reading everything would be painful)
    for (int i = 0; i < k; i++)
        target_file.push_back(L'\0');

    wchar_t c = file.get();

    std::unordered_map<wchar_t, int> target_alphabet_counts;

    while (!file.eof()) {
        target_file.push_back(c);

        target_alphabet_counts.insert({c, 0});
        target_alphabet_counts.at(c)++;

        c = file.get();
    }

    file.close();

    // Copy the last part of the reference file to the beginning
    for (int i = 0; i < k; i++)
        target_file.at(i) = reference_file.at(reference_file.size() - k + i);

    // Remove characters that were in the reference but not in the target
    for (auto it = alphabet_counts.begin(); it != alphabet_counts.end();) {
        auto pair = *it;
        if (target_alphabet_counts.find(pair.first) == target_alphabet_counts.end())
            it = alphabet_counts.erase(it);
        else
            it++;
    }
    
    // Add characters that were in the target but not in the reference
    for (auto& pair : target_alphabet_counts)
        alphabet_counts.insert({pair.first, 1});    // use count at 1 to prevent infinite information

    base_distribution->setBaseDistribution(alphabet_counts);
    for (auto& pair : alphabet_counts)
        probability_distribution[pair.first] = 0;
}

bool CopyModel::eofReference() {
    // We add one because we don't want to predict a character outside of the stream, so we end earlier
    return current_position + 1 >= reference_file.size();
}

bool CopyModel::eofTarget() {
    // We add one because we don't want to predict a character outside of the stream, so we end earlier
    return current_position + 1 >= target_file.size();
}

int CopyModel::countOf(wchar_t c) {
    return alphabet_counts.at(c);
}

double CopyModel::calculateProbability(int hits, int misses) {
    return (hits + alpha) / (hits + misses + 2 * alpha);
}

void CopyModel::setRemainderProbabilities(wchar_t exception, double probability_to_distribute, std::map<wchar_t, double> distribution) {
    double base_remainder_total = 0.0;
    for (auto& pair : distribution)
        if (pair.first != exception)
            base_remainder_total += pair.second;
    
    for (auto& pair : distribution)
        if (pair.first != exception)
            probability_distribution.at(pair.first) = probability_to_distribute * distribution.at(pair.first) / base_remainder_total;
}

double CopyModel::progress() {
    return (double) (current_position + 1) / target_file.size();
}

void CopyModel::guess() {

    actual = target_file.at(current_position + 1);
    
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