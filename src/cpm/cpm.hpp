#include <vector>
#include <string>
#include <unordered_map>
#include <cwchar>
#include <string_view>
#include "copy_pointer_threshold.hpp"
#include "copy_pointer_manager.hpp"
#include "base_distribution.hpp"


/**
 *  \file cpm.hpp (interface file)
 *
 *  \brief Lab1: CPM.
 *
 *  Header file for the CopyModel class.
 * 
 *  Methods defined in this class:
 *      \li registerPattern
 *      \li predictionSetup
 *      \li predict
 *      \li advance
 *      \li firstPassOverReference
 *      \li eof
 *      \li countof
 *      \li guess
 *      \li progress
 *      \li initializeOnReference
 *      \li calculateProbability
 *      \li setRemainderProbabilities
 *      \li surpassedAnyThreshold
 *      
 *  \author Pedro Lima 97860 && Nuno Cunha 98124 && Martinho Tavares 98262
 */

// TODO: put documentation regarding parameters on the parameters themselves
/**
 * @brief Copy Model main class.
 * 
 * This class implements the copy model, which is the main class of the program.
 *  
 * The copy model class contains the following attributes:
 * - k: the size of the pattern to be searched for.
 * 
 * - alpha: a smoothing parameter that is used to calculate the hit probability.
 * 
 * - reading_strategy: a pointer to the reading strategy object, used to read from a file.
 * 
 * - pointer_threshold: a pointer to a copy_pointer_threshold class, which will define the type of threshold used to decide when to reposition the copy pointer.
 * 
 * - pointer_threshold_number: the value of the threshold.
 * 
 * - pointer_manager: a pointer to the copy_pointer_manager class, which will be used to know how/ to which copy pointer we should change to next.
 * 
 * - base_distribution: a pointer to the base_distribution class, which will be used to calculate the base distribution.
 * 
 * - alphabet_counts: a map that contains the number of times each symbol appears in the file. (used to calculate the base distribution)
 * 
 * - current_position: the position of the current pattern in the file.
 * 
 * - current_pattern: the current pattern in the file.
 * 
 * - copy_position: the position of the copy pattern in the file.
 * 
 * - copy_pattern: the copy pattern in the file.
 * 
 * - probability_distribution: the probability distribution of the symbols in the file.
 * 
 * - hit_probability: the hit probability of the prediction.
 * 
 * - prediction: the prediction of the next symbol in the file.
 * 
 * - actual: the actual symbol in the file.
 */

class CopyModel {

    int k;
    double alpha;
    CopyPointerThreshold** pointer_threshold;
    int pointer_threshold_number;
    CopyPointerManager* pointer_manager;
    BaseDistribution* base_distribution;

    std::vector<wchar_t> target_file;
    std::vector<wchar_t> reference_file;
    std::unordered_map<wchar_t, int> alphabet_counts;
    
    size_t current_position = -1;
    std::wstring_view current_pattern;
    size_t copy_position = -1;
    std::wstring_view copy_pattern;
    bool predicting = false;

public:
    CopyModel(int k, double alpha, CopyPointerThreshold** pt, int ptn, CopyPointerManager* pm, BaseDistribution* bd) : 
        k(k), alpha(alpha), pointer_threshold(pt), pointer_threshold_number(ptn), pointer_manager(pm), base_distribution(bd) {}

/**
 * @brief Registers the position of the pattern read in the file to a map (key = pattern, value = array of pointers where that pattern was detected), located in the pointer_manager.
 * 
 * @return true 
 * @return false 
 */
    bool registerPattern();
/**
 * @brief Check if the current pattern has already been registered.
 * 
 * @return true 
 * @return false 
 */
    bool isPatternRegistered();
/**
 * @brief Verifies whether we can make a predction or not.
 * This is done by checking if the copy_pointer isn't pointing to the current position.
 * It also gets the copy pointer from the pointer_manager, if the copy_pointer is pointing to the current position.
 * It also triggers the repositioning of the copy pointer, if the hit probability surpasses the threshold.
 * 
 * @return true 
 * @return false 
 */
    bool predictionSetup(bool);
/**
 * @brief Predicts the next symbol in the file.
 * 
 * This is done by comparing the symbol in the copy position with the symbol in the current position.
 * 
 * @return true 
 * @return false 
 */
    bool predict();
/**
 * @brief Advances the current_position and copy_position in the file, while updating the current_pattern to the one corresponding to the new current_position.
 * 
 *  
 */
    void advance();
/**
 * @brief First read of the file.
 * 
 * This is used to count the number of times each symbol appears in the file, to calculate the base distribution.
 * 
 */
    // TODO: rename this to something more descriptive (what's done in the "first pass"?? there are multiple "first passes"...)
    void firstPassOverReference(std::string);
    void firstPassOverTarget(std::string);
    void updateDistribution();
/**
 * @brief Verifies if the end of the file has been reached.
 * 
 * @return true 
 * @return false 
 */
    bool eofReference();
    bool eofTarget();
/**
 * @brief Returns the number of times a symbol appears in the file.
 * 
 * @return int 
 */
    int countOf(wchar_t);
/**
 * @brief In case we can't make a prediction, we use the base distribution to guess the next symbol.
 * 
 */
    void guess();
/**
 * @brief Returns a percentage of how much of the file has been read.
 * 
 * @return double 
 */
    double progress();
/**
 * @brief Initializes the current_pattern and copy_pattern with a pattern of k size, composed of the most frequent symbol in the file 
 * 
 */
    void initializeOnReference();
    void initializeOnTarget();

    // Read-only values. Always overwritten when calling predictNext()
    std::unordered_map<wchar_t, double> probability_distribution;
    double hit_probability = 0;
    wchar_t prediction = '\0';
    wchar_t actual = '\0';

private:
/**
 * @brief Expression used to calculate the hit probability.
 * 
 * @return double 
 */
    double calculateProbability(int, int);
/**
 * @brief Set the Remainder Probabilities of the probability distribution after the prediction.
 * 
 */
    void setRemainderProbabilities(wchar_t, double, std::unordered_map<wchar_t, double>);
/**
 * @brief Verifies if the hit probability surpasses any threshold, used when the user defined more than one type of threshold for copy pointer changing.
 * 
 * @return true 
 * @return false 
 */
    bool surpassedAnyThreshold(double);

};