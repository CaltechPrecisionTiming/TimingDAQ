#ifndef NetScopeAnalyzer_HH
#define NetScopeAnalyzer_HH
#define NetScope_CHANNELS 4
#define NetScope_TIMES 1
#define NetScope_SAMPLES 1000
#define NetScope_F_SAMPLES 500
#define SCOPE_MEM_LENGTH_MAX 12500000

#include "DatAnalyzer.hh"
#include <assert.h>

// This is the class that should be used for parsing and analyzing
// NetScope data files in .dat format.

class NetScopeAnalyzer : public DatAnalyzer {
  public:
    struct WaveformAttribute
    {
        unsigned int chMask;
        size_t nPt; // number of points in each event
        size_t nFrames; // number of Fast Frames in each event, 0 means off
        float dt;
        float t0;
        float ymult[NetScope_CHANNELS];
        float yoff[NetScope_CHANNELS];
        float yzero[NetScope_CHANNELS];
    };
    //Scope Tektronix DPO7254 ADC already in account in the binary conversion
    NetScopeAnalyzer() : DatAnalyzer(NetScope_CHANNELS, NetScope_TIMES, NetScope_SAMPLES, 1, 1., NetScope_F_SAMPLES) {}

    void GetCommandLineArgs(int argc, char **argv);

    void InitLoop();

    int GetChannelsMeasurement();

    unsigned int GetTimeIndex(unsigned int n_ch) { return 0; }
  protected:
    //NetScope binary
    WaveformAttribute wave_attr;
    vector<int> active_ch = {};
};

#endif
