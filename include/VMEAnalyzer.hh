#ifndef VMEAnalyzer_HH
#define VMEAnalyzer_HH
#define VME_CHANNELS 36
#define VME_SAMPLES 1024

#include "DatAnalyzer.hh"

// This is the class that should be used for parsing and analyzing
// VME data files in .dat format.

class VMEAnalyzer : public DatAnalyzer {
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

    VMEAnalyzer() : DatAnalyzer() {}

    void GetCommandLineArgs(int argc, char **argv);

    void LoadCalibration();

    void InitTree();

    int GetChannelsMeasurement();

    unsigned int GetTimeIndex(unsigned int n_ch) { return n_ch/8; }

    void Analyze();
  protected:
    // Set by command line arguments or default
    TString pixel_input_file_path;
    TString calibration_file_path = "";

    // Calibration vars
    double off_mean[4][9][1024] = {0};
    double tcal[4][1024] = {0};

    // Analysis variables
    float time[VME_CHANNELS][VME_SAMPLES] = {0};  // calibrated time
    float channel[VME_CHANNELS][VME_SAMPLES] = {0};

    // Tree variables
    unsigned short tc[4]; // trigger counter bin
    unsigned short raw[VME_CHANNELS][VME_SAMPLES]; // ADC counts

    // Pixel events variables
    FTBFPixelEvent* pixel_event;
    TFile *pixel_file = nullptr;
    TTree *pixel_tree = nullptr;

    float xIntercept;
    float yIntercept;
    float xSlope;
    float ySlope;
    float x1;
    float y1;
    float x2;
    float y2;
    float chi2;
    int ntracks;

};

#endif
