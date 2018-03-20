#ifndef DatAnalyzer_HH
#define DatAnalyzer_HH

// STD INCLUDES
#include <iostream>
#include <string>

// ROOT INCLUDES
#include "TFile.h"
#include "TTree.h"

// LOCAL INCLUDES
#include "Config.hh"

// This is the base class for .dat --> .root converters.
// It provides two primary functions:
// 1) parse(), which reads a .dat file and returns arrays of
//      raw data in a standard format.  This function is
//      input specific and should be implemented in derived
//      classes.
// 2) analyze(), which takes the arrays from parse(),
//      performs fits, calibrations, etc, and fills an
//      output TTree.  This function
//      is supposed to be agnostic to the original data source.

class DatAnalyzer {
    public:
        DatAnalyzer(std::string configName, int numChannels=36, int numSamples=1024);
        ~DatAnalyzer();
        int getNumChannels() { return NUM_CHANNELS; }
        int getNumSamples() { return NUM_SAMPLES; }

        virtual void parse(std::string inName) {
            std::cerr << "Please use a child class of DatAnalyzer" << std::endl; }
        void analyze(std::string outName);

    private:
        void initTree(std::string fname);

        const int NUM_CHANNELS;
        const int NUM_SAMPLES;
        Config config;

        TFile *file;
        TTree *tree;

        float time[4][1024] = {0};
        short raw[36][1024] = {0};
        // TODO: add all tree variables
};

#endif
