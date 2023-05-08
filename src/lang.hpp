#include <vector>
#include <string>
#include <list>
#include <map>


void outputCurrentInformation(wchar_t prediction, wchar_t actual, double hit_probability, std::map<wchar_t, double> distribution);
void outputProbabilityDistributionHuman(wchar_t prediction, wchar_t actual, double hit_probability, std::map<wchar_t, double> base_distribution);

// Options
void printUsage(char* prog_name);
void printOptions();
