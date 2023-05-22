#include <fstream>
#include <algorithm>
#include <list>
#include "copy_pointer_manager.hpp"

bool SimpleCopyPointerManager::registerCopyPointer(std::wstring_view pattern, size_t position) {
    if (!isPatternRegistered(pattern)) {

        struct SimplePointerInfo pattern_info = {
            .pointers = {position},
            .copy_pointer_index = 0,
        };

        pointer_map[pattern] = pattern_info;
        
        return false;
    }
    
    pointer_map.at(pattern).pointers.push_back(position);
    return true;
}

bool SimpleCopyPointerManager::isPatternRegistered(std::wstring_view pattern) {
    return pointer_map.find(pattern) != pointer_map.end();
}

int SimpleCopyPointerManager::getCopyPointer(std::wstring_view pattern) {
    return pointer_map.at(pattern).pointers[pointer_map.at(pattern).copy_pointer_index];
}

void SimpleCopyPointerManager::reportPrediction(std::wstring_view pattern, bool hit) {
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

int SimpleCopyPointerManager::getHits(std::wstring_view current_pattern) { return hits; }

int SimpleCopyPointerManager::getMisses(std::wstring_view current_pattern) { return misses; }

bool CircularArrayCopyPointerManager::registerCopyPointer(std::wstring_view pattern, size_t position) {
    if (!isPatternRegistered(pattern)) {

        struct CircularArrayPointerInfo pattern_info = {
            .pointers = {position},
            .copy_pointer_index = 0,
            .insertion_point = 1,
        };

        pointer_map[pattern] = pattern_info;
        
        return false;
    }

    if (pointer_map.at(pattern).insertion_point >= array_size) {
        pointer_map.at(pattern).pointers[(pointer_map.at(pattern).insertion_point % array_size)] = position;
    } else {
        pointer_map.at(pattern).pointers.insert(pointer_map.at(pattern).pointers.begin() + pointer_map.at(pattern).insertion_point, position);
    }

    pointer_map.at(pattern).insertion_point++;

    
    return true;
}

bool CircularArrayCopyPointerManager::isPatternRegistered(std::wstring_view pattern) {
    return pointer_map.find(pattern) != pointer_map.end();
}

int CircularArrayCopyPointerManager::getCopyPointer(std::wstring_view pattern) {
    return pointer_map.at(pattern).pointers[pointer_map.at(pattern).copy_pointer_index];
}

void CircularArrayCopyPointerManager::reportPrediction(std::wstring_view pattern, bool hit) {
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

int CircularArrayCopyPointerManager::getHits(std::wstring_view current_pattern) { return hits; }

int CircularArrayCopyPointerManager::getMisses(std::wstring_view current_pattern) { return misses; }

void CircularArrayCopyPointerManager::repositionCopyPointer(std::wstring_view pattern, std::vector<wchar_t>* mem_file) {
    
    // we can consider the last pointer in the pointers array, assuming the current pattern hasn't been added yet
    std::list<size_t> pointer_candidates(pointer_map.at(pattern).pointers.begin(), std::prev(pointer_map.at(pattern).pointers.end()));
    
    int count;
    int offset = 1;
    wchar_t most_frequent = '\0';

    do {
        count = 0;

        // Majority algorithm: first pass (determine most frequent)
        for (std::list<size_t>::iterator it = pointer_candidates.begin(); it != pointer_candidates.end();) {
            size_t pointer = *it;
            // Remove the pointer candidate if it is starting to point outside of the message
            if (pointer + offset < mem_file->size())
                it++;
            else {
                it = pointer_candidates.erase(it);
                continue;
            }
            
            wchar_t char_at_pointer = mem_file->at(pointer + offset);

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
            if ((pointer + offset < mem_file->size()) && mem_file->at(pointer + offset) == most_frequent)
                it++;
            else
                it = pointer_candidates.erase(it);
        }

        offset++;

    } while (count > 1);

    size_t pointer_candidate = pointer_candidates.back();
    unsigned int i;
    for (i = 0; i < pointer_map.at(pattern).pointers.size(); i++)
        if (pointer_map.at(pattern).pointers[i] == pointer_candidate)
            break;
    pointer_map.at(pattern).copy_pointer_index = i;

}

void RecentCopyPointerManager::repositionCopyPointer(std::wstring_view pattern, std::vector<wchar_t>* mem_file) {
    // second to last copy pointer (because most recent could lead to predicting future)
    pointer_map.at(pattern).copy_pointer_index = pointer_map[pattern].pointers.size() - 2;
}

void NextOldestCopyPointerManager::repositionCopyPointer(std::wstring_view pattern, std::vector<wchar_t>* mem_file) {
    pointer_map.at(pattern).copy_pointer_index += 1;
}

void MostCommonCopyPointerManager::repositionCopyPointer(std::wstring_view pattern, std::vector<wchar_t>* mem_file) {
    
    // we can consider the last pointer in the pointers array, assuming the current pattern hasn't been added yet
    std::list<size_t> pointer_candidates(pointer_map[pattern].pointers.begin(), pointer_map[pattern].pointers.end());
    
    int count;
    int offset = 1;
    wchar_t most_frequent = '\0';

    do {
        count = 0;

        // First pass: majority algorithm (determine most frequent)
        for (std::list<size_t>::iterator it = pointer_candidates.begin(); it != pointer_candidates.end();) {
            size_t pointer = *it;
            // Remove the pointer candidate if it is starting to point outside of the message
            if (pointer + offset < mem_file->size())
                it++;
            else {
                it = pointer_candidates.erase(it);
                continue;
            }
            
            wchar_t char_at_pointer = mem_file->at(pointer + offset);

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
            if ((pointer + offset < mem_file->size()) && mem_file->at(pointer + offset) == most_frequent)
                it++;
            else
                it = pointer_candidates.erase(it);
        }

        offset++;

    } while (count > 1);

    size_t pointer_candidate = pointer_candidates.back();
    unsigned int i;
    for (i = 0; i < pointer_map.at(pattern).pointers.size(); i++)
        if (pointer_map.at(pattern).pointers[i] == pointer_candidate)
            break;
    pointer_map.at(pattern).copy_pointer_index = i;

}