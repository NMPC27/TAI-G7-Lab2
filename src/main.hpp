#include <vector>
#include <string>
#include <list>
#include <map>


void outputProbabilityDistributionCSVheader();
void outputProbabilityDistributionCSVbody(char prediction, char actual, double hit_probability, std::map<char, double> distribution);
void outputProbabilityDistributionHuman(char prediction, char actual, double hit_probability, std::map<char, double> base_distribution);

// Options
void printUsage(char* prog_name);
void printOptions();
