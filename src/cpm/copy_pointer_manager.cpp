#include <fstream>
#include <algorithm>
#include <list>
#include "copy_pointer_manager.hpp"

bool SimpleCopyPointerManager::registerCopyPointer(std::wstring_view pattern, size_t position) {
    auto emplace_result = pointer_map.try_emplace(pattern);
    emplace_result.first->second.pointers.push_back(position);
    return emplace_result.second;
}

bool SimpleCopyPointerManager::isPatternRegistered(std::wstring_view pattern) {
    return pointer_map.find(pattern) != pointer_map.end();
}

int SimpleCopyPointerManager::getCopyPointer(std::wstring_view pattern) {
    return pointer_map.at(pattern).pointers.at(pointer_map.at(pattern).copy_pointer_index);
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
    auto emplace_result = pointer_map.try_emplace(pattern);
    auto& pointer_info = emplace_result.first->second;

    if (pointer_info.insertion_point >= array_size) {
        pointer_info.pointers[(pointer_info.insertion_point % array_size)] = position;
    } else {
        pointer_info.pointers.insert(pointer_info.pointers.begin() + pointer_info.insertion_point, position);
    }

    pointer_info.insertion_point++;

    return emplace_result.second;
}

bool CircularArrayCopyPointerManager::isPatternRegistered(std::wstring_view pattern) {
    return pointer_map.find(pattern) != pointer_map.end();
}

int CircularArrayCopyPointerManager::getCopyPointer(std::wstring_view pattern) {
    return pointer_map.at(pattern).pointers.at(pointer_map.at(pattern).copy_pointer_index);
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
        if (pointer_map.at(pattern).pointers.at(i) == pointer_candidate)
            break;
    if (i < pointer_map.at(pattern).pointers.size())
        pointer_map.at(pattern).copy_pointer_index = i;

}

void RecentCopyPointerManager::repositionCopyPointer(std::wstring_view pattern, std::vector<wchar_t>* mem_file) {
    // second to last copy pointer (because most recent could lead to predicting future)
    pointer_map.at(pattern).copy_pointer_index = pointer_map.at(pattern).pointers.size() - 2;
}

void NextOldestCopyPointerManager::repositionCopyPointer(std::wstring_view pattern, std::vector<wchar_t>* mem_file) {
    pointer_map.at(pattern).copy_pointer_index += 1;
}

void MostCommonCopyPointerManager::repositionCopyPointer(std::wstring_view pattern, std::vector<wchar_t>* mem_file) {
    
    // we can consider the last pointer in the pointers array, assuming the current pattern hasn't been added yet
    std::list<size_t> pointer_candidates(pointer_map.at(pattern).pointers.begin(), pointer_map.at(pattern).pointers.end());
    
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
        if (pointer_map.at(pattern).pointers.at(i) == pointer_candidate)
            break;
    if (i < pointer_map.at(pattern).pointers.size())
        pointer_map.at(pattern).copy_pointer_index = i;

}