#include "stdlib.h"
#include <vector>
#ifndef PARSER_HPP
#define PARSER_HPP

/**
 *  \file parser.hpp (interface file)
 *
 *  \brief Lab1: CPM.
 *
 *  Header file for the Reading Strategy classes.
 * 
 *  Classes defined in this file:
 *    \li ReadingStrategy
 *      \li InMemoryReadingStrategy
 *
 *  \author Pedro Lima 97860 && Nuno Cunha 98124 && Martinho Tavares 98262
 */

/**
 * @brief Reading Strategy interface.
 * 
 */
class ReadingStrategy {

public:
    virtual ~ReadingStrategy() {};
    virtual void read(wchar_t) = 0;
    virtual wchar_t at(size_t) = 0;
    virtual size_t endOfStream() = 0;

};

/**
 * @brief Reading Strategy that reads the file into memory.
 * 
 */

class InMemoryReadingStrategy : public ReadingStrategy {

    /**
     * @brief Vector that contains the file in memory.
     * 
     */
    std::vector<wchar_t> mem_file;

public:

    /**
     * @brief Reads a character from the file and stores it in the vector.
     * 
     */
    void read(wchar_t);
    /**
     * @brief Returns the character at the given position in the vector.
     * 
     * @return wchar_t 
     */
    wchar_t at(size_t);
    /**
     * @brief Returns the size of the vector.
     * 
     * @return size_t 
     */
    size_t endOfStream();
    /**
     * @brief Clears the vector.
     * 
     */
    void clean();

};

#endif