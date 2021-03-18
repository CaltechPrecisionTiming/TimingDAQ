#ifndef KeysightAnalyzer_HH
#define KeysightAnalyzer_HH
#define Keysight_CHANNELS 3
#define Keysight_TIMES 3
#define Keysight_SAMPLES 10000

#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cstring>

#include "DatAnalyzer.hh"

// This is the class that should be used for parsing and analyzing
// Keysight data files in .ROOT format.

class KeysightAnalyzer : public DatAnalyzer {
  public:
    KeysightAnalyzer() : DatAnalyzer(Keysight_CHANNELS, Keysight_TIMES, Keysight_SAMPLES, 1., 1.) {}

    // void GetCommandLineArgs(int argc, char **argv);

    void InitLoop();

    int GetChannelsMeasurement(int i);

    unsigned int GetTimeIndex(unsigned int n_ch) { return n_ch; }

    void Analyze();
  protected:
    float event_time[4][1024];
    std::vector<unsigned int> active_channels;

};

#endif
