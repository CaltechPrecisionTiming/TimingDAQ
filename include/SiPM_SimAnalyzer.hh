#ifndef SiPM_SimAnalyzer_HH
#define SiPM_SimAnalyzer_HH
#define SIM_CHANNELS 1
#define SIM_TIMES 1
#define SIM_SAMPLES 7000

#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cstring>

#include "DatAnalyzer.hh"

// This is the class that should be used for parsing and analyzing
// DRS data files in .dat format.

class SiPM_SimAnalyzer : public DatAnalyzer {
  public:
    SiPM_SimAnalyzer() : DatAnalyzer(SIM_CHANNELS, SIM_TIMES, SIM_SAMPLES, 1000., 1.) {}

    // void GetCommandLineArgs(int argc, char **argv);

    void InitLoop();

    int GetChannelsMeasurement();

    unsigned int GetTimeIndex(unsigned int n_ch) { return n_ch; }

    void Analyze();
  protected:
    float event_time[4][1024];
    std::vector<unsigned int> active_channels;
    //sPulseShape ps;

};

#endif
