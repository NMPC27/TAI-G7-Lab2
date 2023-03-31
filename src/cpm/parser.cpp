#include "parser.hpp"

void InMemoryReadingStrategy::read(char c) {
    this->mem_file.push_back(c);
}

char InMemoryReadingStrategy::at(size_t pos) {
    return this->mem_file[pos];
}

size_t InMemoryReadingStrategy::endOfStream() {
    return this->mem_file.size();
}

void InMemoryReadingStrategy::clean() {
    this->mem_file.clear();
}
