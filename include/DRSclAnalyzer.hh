#ifndef DRSclAnalyzer_HH
#define DRSclAnalyzer_HH
#define DRS_CHANNELS 4
#define DRS_TIMES 4
#define DRS_SAMPLES 1024

#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cstring>

#include "DatAnalyzer.hh"

// This is the class that should be used for parsing and analyzing
// DRS data files in .dat format.

class DRSclAnalyzer : public DatAnalyzer {
  public:
    DRSclAnalyzer() : DatAnalyzer(DRS_CHANNELS, DRS_TIMES, DRS_SAMPLES, 1, 1.e-3) {}

    // void GetCommandLineArgs(int argc, char **argv);

    void InitLoop();

    int GetChannelsMeasurement();

    unsigned int GetTimeIndex(unsigned int n_ch) { return n_ch; }

    void Analyze();
  protected:
    float event_time[4][1024];
    std::vector<unsigned int> active_channels;

};

#endif
