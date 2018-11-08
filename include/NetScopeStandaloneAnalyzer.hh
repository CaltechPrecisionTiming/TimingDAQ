#ifndef NetScopeStandaloneAnalyzer_HH
#define NetScopeStandaloneAnalyzer_HH
#define NetScope_CHANNELS 4
#define NetScope_TIMES 1
#define NetScope_SAMPLES 1000
#define NetScope_F_SAMPLES 500
#define SCOPE_MEM_LENGTH_MAX 12500000

#include "DatAnalyzer.hh"
#include <assert.h>

// This is the class that should be used for parsing and analyzing
// NetScope data files in .root format produced by the python script.

class NetScopeStandaloneAnalyzer : public DatAnalyzer {
  public:
    //Scope Tektronix DPO7254 ADC already in account in the binary conversion
    NetScopeStandaloneAnalyzer() : DatAnalyzer(NetScope_CHANNELS, NetScope_TIMES, NetScope_SAMPLES, 1, 1., NetScope_F_SAMPLES) {}

    void GetCommandLineArgs(int argc, char **argv);

    void InitLoop();

    int GetChannelsMeasurement(int i_aux);

    unsigned int GetTimeIndex(unsigned int n_ch) { return 0; }
  protected:
    vector<int> active_ch = {0,1,2,3};
};

#endif
