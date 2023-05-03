#include <vector>
#include <string>
#include <map>
#include <cwchar>

/**
 *  \file base_distribution.hpp (interface file)
 *
 *  \brief Lab1: CPM.
 *
 *  Header file for the BaseDistribution classes.
 *
 *  Classes defined in this file:
 *     \li BaseDistribution
 *     \li UniformDistribution
 *     \li FrequencyDistribution
 *
 *  \author Pedro Lima 97860 && Nuno Cunha 98124 && Martinho Tavares 98262
 */

/**
 * @brief Base class for the distribution classes.
 * 
 */
class BaseDistribution {
public:
/**
 * @brief Set the Base Distribution object
 * 
 * @param histogram 
 * 
 * This method uses the histogram count to set the base distribution.
 */
    virtual ~BaseDistribution() {};
    virtual void setBaseDistribution(std::map<wchar_t, int> histogram) = 0;
    std::map<wchar_t, double> distribution;
};

/**
 * @brief Uniform distribution class.
 * 
 * This class implements the uniform distribution, which means that all the characters have the same probability.
 * 
 */

class UniformDistribution : public BaseDistribution {
public:
    void setBaseDistribution(std::map<wchar_t, int> histogram);
};

/**
 * @brief Frequency distribution class.
 * 
 * This class implements the frequency distribution, which means that the probability of a character is equal to the frequency of that character in all of the input file.
 * 
 */

class FrequencyDistribution : public BaseDistribution {
public:
    void setBaseDistribution(std::map<wchar_t, int> histogram);
};