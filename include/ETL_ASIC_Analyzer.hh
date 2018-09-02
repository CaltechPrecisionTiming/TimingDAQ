#ifndef ETL_ASIC_Analyzer_HH
#define ETL_ASIC_Analyzer_HH
#define ETL_ASIC_CHANNELS 1
#define ETL_ASIC_TIMES 1
#define ETL_ASIC_SAMPLES 5000

#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cstring>

#include "DatAnalyzer.hh"

// This is the class that should be used for parsing and analyzing
// DRS data files in .dat format.

class ETL_ASIC_Analyzer : public DatAnalyzer {
  public:
    ETL_ASIC_Analyzer() : DatAnalyzer(ETL_ASIC_CHANNELS, ETL_ASIC_TIMES, ETL_ASIC_SAMPLES, 1, 1.) {}

    // void GetCommandLineArgs(int argc, char **argv);

    void InitLoop();

    int GetChannelsMeasurement( int i);

    unsigned int GetTimeIndex(unsigned int n_ch) { return n_ch; }

    void Analyze();
  protected:
    float event_time[4][1024];
    std::vector<unsigned int> active_channels;

};

#endif
