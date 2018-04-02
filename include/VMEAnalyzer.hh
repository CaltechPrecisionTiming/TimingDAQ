#ifndef VMEAnalyzer_HH
#define VMEAnalyzer_HH
#define VME_CHANNELS 36
#define VME_TIMES 4
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

    VMEAnalyzer() : DatAnalyzer(VME_CHANNELS, VME_TIMES, VME_SAMPLES, 4096, 1.) {}

    void GetCommandLineArgs(int argc, char **argv);

    void LoadCalibration();

    void InitLoop();

    int FixCorruption();

    int GetChannelsMeasurement();

    unsigned int GetTimeIndex(unsigned int n_ch) { return n_ch/9; }

    void Analyze();
  protected:
    // Set by command line arguments or default
    TString pixel_input_file_path;
    TString calibration_file_path = "";

    // Calibration vars
    double off_mean[4][9][1024];
    double tcal[VME_TIMES][1024];

    // Tree variables
    unsigned short tc[VME_TIMES]; // trigger counter bin
    unsigned short raw[VME_CHANNELS][VME_SAMPLES]; // ADC counts

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
