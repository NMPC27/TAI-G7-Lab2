#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
#include <string>
#include <list>
#include <vector>
#include <cmath>
#include <sys/stat.h>
#include <unistd.h>
#include <chrono>

#include "cpm/cpm.hpp"
#include "main.hpp"

using namespace std;

/** @brief Maximum number of unique copy pointers */
#define POINTER_THRESHOLD_MAX_NUMBER 3
#define POINTER_THRESHOLD_MASK_STATIC 1
#define POINTER_THRESHOLD_MASK_DERIVATIVE 2
#define POINTER_THRESHOLD_MASK_SUCCESSIVE_FAILS 4

/**
 *  \file main.cpp (implementation file)
 *
 *  \brief Lab1: CPM.
 *
 *  This file instantiates the CPM model and runs it.
 * 
 *  \author Pedro Lima 97860 && Nuno Cunha 98124 && Martinho Tavares 98262
 */

int main(int argc, char** argv) {

    chrono::steady_clock sc;   // create an object of `steady_clock` class
    auto start = sc.now();     // start timer

    // Default values
    int c;
    int k = 12;
    double alpha = 1.0;
    enum VerboseMode{human, machine, progress, none} verbose_mode = VerboseMode::none;
    ReadingStrategy* reading_strategy = nullptr;
    CopyPointerThreshold* pointer_thresholds[POINTER_THRESHOLD_MAX_NUMBER];
    int pointer_threshold_number = 0;
    int pointer_threshold_mask = 0;
    CopyPointerManager* pointer_manager = nullptr;
    BaseDistribution* base_distribution = nullptr;

    // Parse command line arguments
    while ((c = getopt(argc, argv, "hv:bk:a:p:r:t:")) != -1){
        switch(c){
            // Help
            case 'h':
                printUsage(argv[0]);
                printOptions();
                return 0;
            /* Verbose mode
             * h: human
             * m: machine
             * p: progress
            */ 
            case 'v':
                switch (optarg[0]) {
                    case 'h':
                        verbose_mode = VerboseMode::human;
                        break;
                    case 'm':
                        verbose_mode = VerboseMode::machine;
                        break;
                    case 'p':
                        verbose_mode = VerboseMode::progress;
                        break;
                    default:
                        cout << "Error: invalid option for '-v' (" << optarg[0] << ")" << endl;
                        return 1;
                }
                break;
            
            // K (size of the pattern)
            case 'k':
                k = stoi(optarg);
                break;

            // Alpha (smoothing factor)
            case 'a':
                alpha = stof(optarg);
                break;

            // base distribution : u (uniform) or f (frequency)
            case 'p':
                if (optarg[0] == 'u') {
                    base_distribution = new UniformDistribution(); 
                } else if (optarg[0] == 'f') {
                    base_distribution = new FrequencyDistribution();
                } else {
                    cout << "Error: invalid option for '-p' (" << optarg[0] << ")" << endl;
                    return 1;
                }
                break;

            /*Pointer manager options

            o: next oldest pointer
            n: recent pointer
            m: most common pointer
            c:X circular array with X elements and most common pointer strategy
            */
            case 'r':
                {
                string optarg_string = string(optarg);
                int pos = optarg_string.find(":");

                string opt = optarg_string.substr(0, pos);
                string value = optarg_string.substr(pos+1, optarg_string.length());

                if (opt == "o") {
                    pointer_manager = new NextOldestCopyPointerManager();
                } else if (opt == "n") {
                    pointer_manager = new RecentCopyPointerManager();
                } else if (opt == "m") {
                    pointer_manager = new MostCommonCopyPointerManager();
                } else if (opt == "c"){

                    int circular_value = stoi(value);
                    if (circular_value > 0){
                        pointer_manager = new CircularArrayCopyPointerManager(circular_value);
                    }else{
                        cout << "Error: invalid option for '-r c:X' (" << optarg << ")" << endl;
                        return 1;
                    }
                }else {
                    cout << "Error: invalid option for '-r' (" << optarg[0] << ")" << endl;
                    return 1;
                }
                }
                break;
            
            /* Pointer Threshold
            
            n:X static threshold with value X
            f:X successive fails threshold with value X
            c:X derivative threshold with value X
            */
            case 't':
                {
                    string optarg_string = string(optarg);
                    int pos = optarg_string.find(":");
                    if (pos == -1) {
                        cout << "Error: invalid option for '-t' (" << optarg << ")" << endl;
                        return 1;
                    }

                    string opt = optarg_string.substr(0, pos);
                    string value = optarg_string.substr(pos+1, optarg_string.length());

                    if (opt == "n") {
                        if (pointer_threshold_mask & POINTER_THRESHOLD_MASK_STATIC) {
                            cout << "Error: mode '" << opt << "' for option '-t' was specified more than once (repeated value '" << optarg << "')" << endl;
                            return 1;
                        }
                        double threshold_value = stof(value);
                        pointer_thresholds[pointer_threshold_number] = new StaticCopyPointerThreshold(threshold_value);
                        pointer_threshold_number++;
                        pointer_threshold_mask |= POINTER_THRESHOLD_MASK_STATIC;
                        
                    } else if (opt == "f") {
                        if (pointer_threshold_mask & POINTER_THRESHOLD_MASK_SUCCESSIVE_FAILS) {
                            cout << "Error: mode '" << opt << "' for option '-t' was specified more than once (repeated value '" << optarg << "')" << endl;
                            return 1;
                        }
                        int threshold_value = stoi(value);
                        if (threshold_value > 0){
                            pointer_thresholds[pointer_threshold_number] = new SuccessFailsCopyPointerThreshold(threshold_value);
                            pointer_threshold_number++;
                            pointer_threshold_mask |= POINTER_THRESHOLD_MASK_SUCCESSIVE_FAILS;
                        } else {
                            cout << "Error: invalid option for '-t f:X' (" << optarg << ")" << endl;
                            return 1;
                        }
                        
                    } else if (opt == "c") {
                        if (pointer_threshold_mask & POINTER_THRESHOLD_MASK_DERIVATIVE) {
                            cout << "Error: mode '" << opt << "' for option '-t' was specified more than once (repeated value '" << optarg << "')" << endl;
                            return 1;
                        }
                        double threshold_value = stof(value);
                        pointer_thresholds[pointer_threshold_number] = new DerivativeCopyPointerThreshold(threshold_value);
                        pointer_threshold_number++;
                        pointer_threshold_mask |= POINTER_THRESHOLD_MASK_DERIVATIVE;
                    } else {
                        cout << "Error: invalid option for '-t' (" << optarg << ")" << endl;
                        return 1;
                    }
                }
                break;                

            case '?':
                printUsage(argv[0]);
                return 1;
        }
    }

    if (optind == argc) {
        printUsage(argv[0]);
        cout << "Error: no file was specified!" << endl;
        return 1;
    }

    // Defaults
    if (reading_strategy == nullptr) reading_strategy = new InMemoryReadingStrategy();
    if (pointer_threshold_number == 0) {
        pointer_thresholds[pointer_threshold_number] = new SuccessFailsCopyPointerThreshold(6);
        pointer_threshold_number++;
    }
    if (pointer_manager == nullptr) pointer_manager = new MostCommonCopyPointerManager();
    if (base_distribution == nullptr) base_distribution = new FrequencyDistribution();

    // Copy model initialization
    CopyModel model = CopyModel(k, alpha, reading_strategy, pointer_thresholds, pointer_threshold_number, pointer_manager, base_distribution);

    string file_name = string(argv[optind]);

    struct stat file_status;
    stat(file_name.c_str(), &file_status);
    if (errno == ENOENT) {
        cout << "Error: file '" << file_name << "' doesn't exist!" << endl;
        return 1;
    }

    // First pass of the file to compute the base distribution
    model.firstPass(file_name);

    map<char, double> information_sums;

    // Initialize the first k-pattern with the most frequent symbol
    model.initializeWithMostFrequent();

    if (verbose_mode == VerboseMode::machine)
        outputProbabilityDistributionCSVheader();

    // Loop for prediction through the file
    while (!model.eof()) {

        // Register the current pattern and check if it has been seen before
        bool pattern_has_past = model.registerPattern();
        // Check if the model can predict the next symbol
        bool can_predict = model.predictionSetup(pattern_has_past);

        int output_color_condition = can_predict ? 1 : 0;
        // If the model can predict, then predict and check if the prediction was correct
        if (can_predict) {
            bool hit = model.predict();
            output_color_condition += hit ? 1 : 0;
        // If the model can't predict, then guess
        } else {
            model.guess();
        }
        // Advance the positions of the current pointer, as well as the current pattern, and the copy pointer
        model.advance();

        // The probability distribution that the model provides doesn't account for whether or not the current prediction was a success,
        // as that would incorporate information from the future which would not be known to the decoder.
        switch (verbose_mode) {
            case VerboseMode::human:
                {
                    string output_color;
                    switch (output_color_condition) {
                        // New pattern, doesn't exist
                        case 0:
                            output_color = "\e[0;33m";
                            break;
                        // Pattern exists, but no hit
                        case 1:
                            output_color = "\e[0;31m";
                            break;
                        // Pattern exists, with hit
                        case 2:
                            output_color = "\e[0;32m";
                            break;
                    }
                    cout << output_color;
                    outputProbabilityDistributionHuman(model.prediction, model.actual, model.hit_probability, model.probability_distribution);
                    cout << "\e[0m";
                }
                break;
            case VerboseMode::machine:
                outputProbabilityDistributionCSVbody(model.prediction, model.actual, model.hit_probability, model.probability_distribution);
                break;
            case VerboseMode::progress:
                printf("Progress: %3f%%\r", model.progress() * 100);
                break;
            case VerboseMode::none:
                break;
        }
        information_sums[model.actual] += -log2(model.probability_distribution[model.actual]);
    }

    delete reading_strategy;
    for (int i = 0; i < pointer_threshold_number; i++)
        delete pointer_thresholds[i];
    delete pointer_manager;
    delete base_distribution;

    double information_sum = 0.0;
    cout << "Average amount of information in symbol..." << endl;
    for (auto pair : information_sums) {
        cout << pair.first << ": " << pair.second / model.countOf(pair.first) << " bits" << endl;
        information_sum += pair.second;
    }

    int sum=0;
    for(std::map<char,double>::iterator it = model.probability_distribution.begin(); it != model.probability_distribution.end(); ++it) {
        sum+=model.countOf(it->first);
    }
    cout << "Mean amount of information of a symbol: " << information_sum/sum << " bits" << endl;
    cout << "Total amount of information: " << information_sum << " bits" << endl;

    auto end = sc.now();
    auto time_span = static_cast<chrono::duration<double>>(end - start);   // measure time span between start & end

    cout << "Time elapsed: " << time_span.count() << " seconds" << endl;

    return 0;
}

void outputProbabilityDistributionHuman(char prediction, char actual, double hit_probability, map<char, double> base_distribution) {
    cout << "Prediction: '" << prediction << "', Actual: '" << actual << "', " << hit_probability << "\t" << " | Distribution: ";
    for (auto pair : base_distribution) {
        cout << "('" << pair.first << "', " << pair.second << ") ";
    }
    cout << endl;
}

void outputProbabilityDistributionCSVheader() {
    cout << "Prediction, Actual, Hit probability, Distribution" << endl;
}

void outputProbabilityDistributionCSVbody(char prediction, char actual, double hit_probability, map<char, double> distribution) {
    cout << prediction << "," << actual << "," << hit_probability << ",";
    for (auto pair : distribution) {
        cout << pair.first << "\t" << pair.second << "\t";
    }
    cout << endl;
}

void printUsage(char* prog_name) {
    cout << "Usage: " << prog_name << " [OPTIONS] file" << endl;
}

void printOptions() {
    cout << "Options:" << endl;
    cout << "\t-h\t\tShow this help message" << endl;
    cout << "\t-v V\t\tAdditional output (verbose modes output the probability distribution at each encoding step):" << endl;
    cout << "\t\t\t\th - Human-readable verbose output, color-coded depending on whether a hit/miss/guess occurred" << endl;
    cout << "\t\t\t\tm - Machine-readable verbose output, without color-coding and minimal flair (CSV format with header)" << endl;
    cout << "\t\t\t\tp - Print the progress of processing the sequence" << endl;
    cout << "\t-k K\t\tSize of the sliding window (default: 12)" << endl;
    cout << "\t-a A\t\tSmoothing parameter alpha for the prediction probability (default: 1.0)" << endl;
    cout << "\t-p P\t\tProbability distribution of the characters other than the one being predicted (default: f):" << endl;
    cout << "\t\t\t\tu - uniform distribution" << endl;
    cout << "\t\t\t\tf - distribution based on the symbols' relative frequencies" << endl;
    cout << "\t-r R\t\tCopy pointer reposition (default: m):" << endl;
    cout << "\t\t\t\to - oldest" << endl;
    cout << "\t\t\t\tn - newer" << endl;
    cout << "\t\t\t\tm - most common prediction among all pointers" << endl;
    cout << "\t-t T\t\tThreshold for copy pointer switch (default: f:6):" << endl;
    cout << "\t\t\t\tn:X - static probability below X" << endl;
    cout << "\t\t\t\tf:X - number of successive fails above X" << endl; //! temos de ver que o numero faz sentido
    cout << "\t\t\t\tc:X - absolute value of the negative derivative of the prediction probability above X" << endl;
}
