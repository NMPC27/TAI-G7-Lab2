#include "parser.hpp"

void InMemoryReadingStrategy::read(wchar_t c) {
    this->mem_file.push_back(c);
}

wchar_t InMemoryReadingStrategy::at(size_t pos) {
    return this->mem_file[pos];
}

size_t InMemoryReadingStrategy::endOfStream() {
    return this->mem_file.size();
}

void InMemoryReadingStrategy::clean() {
    this->mem_file.clear();
}
