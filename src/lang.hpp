#include <vector>
#include <string>
#include <list>
#include <unordered_map>


void outputProbabilityDistributionHuman(wchar_t prediction, wchar_t actual, double hit_probability, std::unordered_map<wchar_t, double> base_distribution);

// Options
void printUsage(char* prog_name);
void printOptions();
