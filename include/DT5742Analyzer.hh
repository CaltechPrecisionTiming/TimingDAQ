#ifndef DT5742Analyzer_HH
#define DT5742Analyzer_HH
#define DT5742_CHANNELS 18
#define DT5742_TIMES 2
#define DT5742_SAMPLES 1024
#define DT5742_FREQ 1 //in GHz, 0 is 5, 1 is 2.5 and 2 is 1 for DT5742 config

#include "DatAnalyzer.hh"
#include <assert.h>

// This is the class that should be used for parsing and analyzing
// DT5742 data files in .dat format.

class DT5742Analyzer : public DatAnalyzer {
  public:
    struct FTBFPixelEvent {
        double xSlope;
        double ySlope;
        double xIntercept;
        double yIntercept;
        double chi2;
        int trigger;
        int runNumber;
        Long64_t timestamp;
    };

    DT5742Analyzer() : DatAnalyzer(DT5742_CHANNELS, DT5742_TIMES, DT5742_SAMPLES, 4096, 1.) {}

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
    double tcal[DT5742_TIMES][1024];

    //DT5742 binary
    unsigned int triggerNumber = -1;
    unsigned short N_corr = 0;
    unsigned long Max_corruption = 0;
    unsigned int event_time_tag = 0;
    unsigned int group_time_tag = 0;

    unsigned int ref_event_size = 0;
    unsigned int N_false = 0;

    vector<int> manual_skip = {0};

    // Tree variables
    unsigned short tc[DT5742_TIMES]; // trigger counter bin
    unsigned short raw[DT5742_CHANNELS][DT5742_SAMPLES]; // ADC counts
    long lSize;
    float* buffer;
    size_t result;

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
