#include <fstream>
#include <algorithm>
#include <list>
#include "copy_pointer_manager.hpp"

bool SimpleCopyPointerManager::registerCopyPointer(std::string pattern, size_t position) {
    if (pointer_map.count(pattern) == 0) {

        struct SimplePointerInfo pattern_info = {
            .pointers = {position},
            .copy_pointer_index = 0,
        };

        pointer_map.insert({pattern, pattern_info});
        
        return false;
    }
    
    pointer_map[pattern].pointers.push_back(position);
    return true;
}

bool SimpleCopyPointerManager::isPatternRegistered(std::string pattern) {
    return pointer_map.count(pattern) != 0;
}

int SimpleCopyPointerManager::getCopyPointer(std::string pattern) {
    return pointer_map[pattern].pointers[pointer_map[pattern].copy_pointer_index];
}

void SimpleCopyPointerManager::reportPrediction(std::string pattern, bool hit) {
    if (hit) {
        hits++;
    } else {
        misses++;
    }
}

void SimpleCopyPointerManager::reset() {
    hits = 0;
    misses = 0;
}

int SimpleCopyPointerManager::getHits(std::string current_pattern) { return hits; }

int SimpleCopyPointerManager::getMisses(std::string current_pattern) { return misses; }

bool CircularArrayCopyPointerManager::registerCopyPointer(std::string pattern, size_t position) {
    if (pointer_map.count(pattern) == 0) {

        struct CircularArrayPointerInfo pattern_info = {
            .pointers = {position},
            .copy_pointer_index = 0,
            .insertion_point = 1,
        };

        pointer_map.insert({pattern, pattern_info});
        
        return false;
    }

    if (pointer_map[pattern].insertion_point >= array_size)
    {
        pointer_map[pattern].pointers[(pointer_map[pattern].insertion_point % array_size)] = position;
    }else{
        pointer_map[pattern].pointers.insert(pointer_map[pattern].pointers.begin() + pointer_map[pattern].insertion_point, position);
    }

    pointer_map[pattern].insertion_point++;

    
    return true;
}

bool CircularArrayCopyPointerManager::isPatternRegistered(std::string pattern) {
    return pointer_map.count(pattern) != 0;
}

int CircularArrayCopyPointerManager::getCopyPointer(std::string pattern) {
    return pointer_map[pattern].pointers[pointer_map[pattern].copy_pointer_index];
}

void CircularArrayCopyPointerManager::reportPrediction(std::string pattern, bool hit) {
    if (hit) {
        hits++;
    } else {
        misses++;
    }
}

void CircularArrayCopyPointerManager::reset() {
    hits = 0;
    misses = 0;
}

int CircularArrayCopyPointerManager::getHits(std::string current_pattern) { return hits; }

int CircularArrayCopyPointerManager::getMisses(std::string current_pattern) { return misses; }

void CircularArrayCopyPointerManager::repositionCopyPointer(std::string pattern, ReadingStrategy* reading_strategy) {
    
    // we should not consider the last pointer in the pointers array, since it's the one that was just added
    std::list<size_t> pointer_candidates(pointer_map[pattern].pointers.begin(), std::prev(pointer_map[pattern].pointers.end()));
    
    //printf("pointer_candidates.size(): %d",pointer_candidates.size());

    int count;
    int offset = 1;
    char most_frequent = '\0';

    while (most_frequent == '\0' || count > 1){

        count = 0;

        // Majority algorithm: first pass (determine most frequent)
        for (size_t pointer : pointer_candidates) {
            char char_at_pointer = reading_strategy->at(pointer + offset);

            if (count == 0) {
                most_frequent = char_at_pointer;
                count++;
            } else if (most_frequent == char_at_pointer) {
                count++;
            } else {
                count--;
            }
        }

        // Majority algorithm: second pass (remove all pointers that don't match the most frequent)
        for (std::list<size_t>::iterator it = pointer_candidates.begin(); it != pointer_candidates.end();) {
            size_t pointer = *it;
            if (reading_strategy->at(pointer + offset) == most_frequent)
                it++;
            else
                it = pointer_candidates.erase(it);
        }

        offset++;
    }

    size_t pointer_candidate = pointer_candidates.back();
    unsigned int i;
    for (i = 0; i < pointer_map[pattern].pointers.size(); i++)
        if (pointer_map[pattern].pointers[i] == pointer_candidate)
            break;
    pointer_map[pattern].copy_pointer_index = i;

}

void RecentCopyPointerManager::repositionCopyPointer(std::string pattern, ReadingStrategy* reading_strategy) {
    // second to last copy pointer (because most recent could lead to predicting future)
    pointer_map[pattern].copy_pointer_index = pointer_map[pattern].pointers.size() - 2;
}

void NextOldestCopyPointerManager::repositionCopyPointer(std::string pattern, ReadingStrategy* reading_strategy) {
    pointer_map[pattern].copy_pointer_index += 1;
}

void MostCommonCopyPointerManager::repositionCopyPointer(std::string pattern, ReadingStrategy* reading_strategy) {
    
    // we should not consider the last pointer in the pointers array, since it's the one that was just added
    std::list<size_t> pointer_candidates(pointer_map[pattern].pointers.begin(), std::prev(pointer_map[pattern].pointers.end()));
    
    int count;
    int offset = 1;
    char most_frequent = '\0';

    while (most_frequent == '\0' || count > 1){

        count = 0;

        // First pass: majority algorithm (determine most frequent)
        for (size_t pointer : pointer_candidates) {
            char char_at_pointer = reading_strategy->at(pointer + offset);

            if (count == 0) {
                most_frequent = char_at_pointer;
                count++;
            } else if (most_frequent == char_at_pointer) {
                count++;
            } else {
                count--;
            }
        }

        // Second pass (remove all pointers that don't match the most frequent)
        for (std::list<size_t>::iterator it = pointer_candidates.begin(); it != pointer_candidates.end();) {
            size_t pointer = *it;
            if (reading_strategy->at(pointer + offset) == most_frequent)
                it++;
            else
                it = pointer_candidates.erase(it);
        }

        offset++;
    }

    size_t pointer_candidate = pointer_candidates.back();
    unsigned int i;
    for (i = 0; i < pointer_map[pattern].pointers.size(); i++)
        if (pointer_map[pattern].pointers[i] == pointer_candidate)
            break;
    pointer_map[pattern].copy_pointer_index = i;

}