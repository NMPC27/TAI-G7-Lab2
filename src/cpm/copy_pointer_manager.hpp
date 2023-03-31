#include <vector>
#include <string>
#include <map>
#include "parser.hpp"

/**
 *  \file copy_pointer_manager.hpp (interface file)
 *
 *  \brief Lab1: CPM.
 *
 *  Header file for the CopyPointerManager classes.
 * 
 *  Struct defined in this file:
 *    \li SimplePointerInfo
 * 
 *  Classes defined in this file:
 *      \li CopyPointerManager
 *      \li SimpleCopyPointerManager
 *      \li MostCommonCopyPointerManager
 *      \li RecentCopyPointerManager
 *      \li NextOldestCopyPointerManager
 *
 *  \author Pedro Lima 97860 && Nuno Cunha 98124 && Martinho Tavares 98262
 */


/**
 * @brief Struct that holds the pointers of a pattern and which pointer is being used to copy.
 * 
 */
struct SimplePointerInfo {
    std::vector<size_t> pointers;
    int copy_pointer_index;     // Pattern's last symbol
};

struct CircularArrayPointerInfo {
    std::vector<size_t> pointers;
    int copy_pointer_index;     // Pattern's last symbol
    int insertion_point;        
};

/**
 * @brief Abstract class that defines the interface for the CopyPointerManager classes.
 * 
 * The CopyPointerManager classes are used to manage the copy pointers of the patterns.
 * The different classes implement different strategies for this.
 */
class CopyPointerManager {
public:
    virtual ~CopyPointerManager() {};
/**
 * @brief Get the Copy Pointer from a pattern.
 * 
 * @return int 
 */
    virtual int getCopyPointer(std::string) = 0;
/**
 * @brief Reposition the copy pointer of a pattern.
 * 
 * This method will be overwritten by the classes that inherit from this class, which are the different strategies we implemented for pointer reposition.
 * 
 * @param pattern
 * @param reading_strategy
 * 
 */
    virtual void repositionCopyPointer(std::string, ReadingStrategy*) = 0;
/**
 * @brief Register a new copy pointer for a pattern.
 * 
 * @param pattern
 * @param pointer
 * 
 * @return true 
 * @return false 
 */
    virtual bool registerCopyPointer(std::string, size_t) = 0;

/**
 * @brief checks if the prediction was correct and updates the hits and misses.
 * 
 * @param pattern
 * @param hit
 * 
 */
    virtual void reportPrediction(std::string, bool) = 0;
/**
 * @brief resets the hits and misses.
 * 
 * 
 */
    virtual void reset() = 0;
/**
 * @brief Get the Hits of a pattern.
 * 
 * @param pattern
 * 
 * @return int 
 */
    virtual int getHits(std::string) = 0;

/**
 * @brief Get the Misses of a pattern.
 * 
 * @param pattern
 * 
 * @return int 
 */
    virtual int getMisses(std::string) = 0;
};

/**
 * @brief Class that implements the generic functions for all different aproaches.
 * 
 */
class SimpleCopyPointerManager : public CopyPointerManager {

protected:
    std::map<std::string, SimplePointerInfo> pointer_map;
    int hits = 0;
    int misses = 0;

public:
    virtual ~SimpleCopyPointerManager() {};
    int getCopyPointer(std::string);
    bool registerCopyPointer(std::string, size_t);
    void reportPrediction(std::string, bool);
    void reset();
    int getHits(std::string);
    int getMisses(std::string);
};

class CircularArrayCopyPointerManager : public CopyPointerManager {


    std::map<std::string, CircularArrayPointerInfo> pointer_map;
    int hits = 0;
    int misses = 0;
    int array_size;

public:
    CircularArrayCopyPointerManager(int size) : array_size(size) {}
    int getCopyPointer(std::string);
    bool registerCopyPointer(std::string, size_t);
    void reportPrediction(std::string, bool);
    void reset();
    int getHits(std::string);
    int getMisses(std::string);
    void repositionCopyPointer(std::string, ReadingStrategy*);
};

/**
 * @brief Class that implements the Most Common strategy.
 * 
 * This strategy will reposition the copy pointer to a pointer that has the most common symbol after the pattern.
 * This is done by comparing the next symbol of each pointer and choosing the one that has the most common symbol for each iteration.
 * At the end of the loop, a single pointer will be left, which is the one that has the most common symbol after the pattern.
 */

class MostCommonCopyPointerManager : public SimpleCopyPointerManager {
public:
    void repositionCopyPointer(std::string, ReadingStrategy*);    
};

/**
 * @brief Class that implements the Recent strategy.
 * 
 * This strategy will reposition the copy pointer to the most recent pointer of that pattern.
 */
class RecentCopyPointerManager : public SimpleCopyPointerManager {
public:
    void repositionCopyPointer(std::string, ReadingStrategy*);
};

/**
 * @brief Class that implements the Next Oldest strategy.
 * 
 * This strategy will reposition the copy pointer to the next oldest pointer of that pattern.
 */
class NextOldestCopyPointerManager : public SimpleCopyPointerManager {
public:
    void repositionCopyPointer(std::string, ReadingStrategy*);
};