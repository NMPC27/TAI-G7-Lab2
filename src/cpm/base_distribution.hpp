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
    virtual void setParameters(double alpha) {};
    virtual void setBaseDistribution(std::map<wchar_t, int> histogram) = 0;
    virtual void updateWithContext(std::wstring context, wchar_t symbol) {};
    virtual std::map<wchar_t, double> getDistributionWithContext(std::wstring context) = 0;
};

/**
 * @brief Uniform distribution class.
 * 
 * This class implements the uniform distribution, which means that all the characters have the same probability.
 * 
 */

class UniformDistribution : public BaseDistribution {

    std::map<wchar_t, double> distribution;

public:
    void setBaseDistribution(std::map<wchar_t, int> histogram);
    std::map<wchar_t, double> getDistributionWithContext(std::wstring context);
};

/**
 * @brief Frequency distribution class.
 * 
 * This class implements the frequency distribution, which means that the probability of a character is equal to the frequency of that character in all of the input file.
 * 
 */

class FrequencyDistribution : public BaseDistribution {

    std::map<wchar_t, double> distribution;

public:
    void setBaseDistribution(std::map<wchar_t, int> histogram);
    std::map<wchar_t, double> getDistributionWithContext(std::wstring context);
};

/**
 * @brief Finite context distribution class.
 * 
 * This class implements a frequency distribution taking a context of k symbols into account.
 * 
 */

class FiniteContextDistribution : public BaseDistribution {

    std::map<std::wstring, std::map<wchar_t, double>> context_table;
    std::vector<wchar_t> alphabet;
    double alpha;

public:
    void setParameters(double alpha);
    void setBaseDistribution(std::map<wchar_t, int> histogram);
    void updateWithContext(std::wstring context, wchar_t symbol);
    std::map<wchar_t, double> getDistributionWithContext(std::wstring context);
};