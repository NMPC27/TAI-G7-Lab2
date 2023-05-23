#include <vector>
#include <string>

/**
 *  \file copy_pointer_threshold.hpp (interface file)
 *
 *  \brief Lab1: CPM.
 *
 *  Header file for the CopyPointerThreshold classes.
 * 
 *  Classes defined in this file:
 *      \li CopyPointerThreshold
 *      \li StaticCopyPointerThreshold
 *      \li DerivativeCopyPointerThreshold
 *      \li SuccessFailsCopyPointerThreshold
 *
 *  \author Pedro Lima 97860 && Nuno Cunha 98124 && Martinho Tavares 98262
 */


/**
 * @brief Abstract class that defines the interface for the CopyPointerThreshold classes.
 *  The CopyPointerThreshold classes are used to determine if the copy pointer should be repositioned.
 *  The different classes implement different strategies for this.
 */
class CopyPointerThreshold {
public:
    virtual ~CopyPointerThreshold() {};
/**
 * @brief Uses the hit probability to determine if the copy pointer should be repositioned. Will be overwritten by the classes that inherit from this class, which are the different strategies we implemented for this.
 * 
 * @param hit_probability 
 * @return true 
 * @return false 
 */
    virtual bool surpassedThreshold(double hit_probability) = 0;
/**
 * @brief Resets the state of the class. Slightly different for each class that inherits from this one.
 * 
 */
    virtual void reset() = 0;
};

/**
 * @brief Class that implements the static threshold strategy.
 * 
 * The copy pointer is repositioned if the hit probability is lower than the static threshold.
 * 
 * @param st 
 * 
 */

class StaticCopyPointerThreshold : public CopyPointerThreshold {

    double static_threshold;

public:
    StaticCopyPointerThreshold(double st) : static_threshold(st) {}
    bool surpassedThreshold(double hit_probability);
    void reset();
};

/**
 * @brief Class that implements the derivative threshold strategy.
 * 
 * The copy pointer is repositioned if the difference of the hit probability minus the previous hit probability is lower than the derivative threshold.
 * The previous hit probability is updated after the repositioning.
 * 
 * @param dt 
 * 
 */

class DerivativeCopyPointerThreshold : public CopyPointerThreshold {

    double previous_hit_probability = -1;
    double derivative_threshold;

public:
    DerivativeCopyPointerThreshold(double dt) : derivative_threshold(dt) {}
    bool surpassedThreshold(double hit_probability);
    void reset();
};

/**
 * @brief Class that implements the successive fails threshold strategy.
 * 
 * The copy pointer is repositioned if there are a number of successive fails higher than the fails threshold.
 * If there is a hit, the number of successive fails is reduced by one, caped at zero.
 * The number of successive fails is reset after the repositioning.
 * 
 * @param dt 
 * 
 */
class SuccessFailsCopyPointerThreshold : public CopyPointerThreshold {

    double previous_hit_probability = -1;
    int fails_count = 0;
    int fails_threshold;

public:
    SuccessFailsCopyPointerThreshold(double dt) : fails_threshold(dt) {}
    bool surpassedThreshold(double hit_probability);
    void reset();
};