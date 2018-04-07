#ifndef NetScopeAnalyzer_HH
#define NetScopeAnalyzer_HH
#define NetScope_CHANNELS 4
#define NetScope_TIMES 1
#define NetScope_SAMPLES 1024

#include "DatAnalyzer.hh"
#include <assert.h>

// This is the class that should be used for parsing and analyzing
// NetScope data files in .dat format.

class NetScopeAnalyzer : public DatAnalyzer {
  public:
    NetScopeAnalyzer() : DatAnalyzer(NetScope_CHANNELS, NetScope_TIMES, NetScope_SAMPLES, 4096, 1.) {}

    void GetCommandLineArgs(int argc, char **argv);

    void LoadCalibration();

    void InitLoop();

    int FixCorruption(int);

    int GetChannelsMeasurement();

    unsigned int GetTimeIndex(unsigned int n_ch) { return n_ch/9; }

    void Analyze();
  protected:
    // Set by command line arguments or default
    TString pixel_input_file_path;
    TString calibration_file_path = "";

    // Calibration vars
    double off_mean[4][9][1024];
    double tcal[NetScope_TIMES][1024];

    //NetScope binary
    unsigned short N_corr = 0;
    unsigned long Max_corruption = 10;
    unsigned int event_time_tag = 0;
    unsigned int group_time_tag = 0;

    unsigned int ref_event_size = 0;

    vector<int> manual_skip = {0};

    // Tree variables
    unsigned short tc[NetScope_TIMES]; // trigger counter bin
    unsigned short raw[NetScope_CHANNELS][NetScope_SAMPLES]; // ADC counts

    // Pixel events variables
    FTBFPixelEvent* pixel_event;
    TFile *pixel_file = nullptr;
    TTree *pixel_tree = nullptr;

    unsigned long int idx_px_tree = 0;
    unsigned long int entries_px_tree = 0;

    float xIntercept;
    float yIntercept;
    float xSlope;
    float ySlope;
    vector<float> x_DUT;
    vector<float> y_DUT;
    float chi2;
    int ntracks;

};

#endif
