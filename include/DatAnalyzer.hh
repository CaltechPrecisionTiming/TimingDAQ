#ifndef DatAnalyzer_HH
#define DatAnalyzer_HH

// STD INCLUDES
#include <iostream>
#include <string>

// SYS includes
#include <sys/types.h>
#include <sys/stat.h>

// ROOT INCLUDES
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TText.h"
#include "TGraphErrors.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TVectorF.h"
#include "TMatrixF.h"
#include <TComplex.h>
// #include "TDecompSVD.h"
#include "TDecompChol.h"
// #include "TMatrixDSym.h"
#include "TF1.h"

// LOCAL INCLUDES
#include "Configuration.hh"
#include "Interpolator.hh"

#define F_LOW 0//low frequency in GHz
#define F_HIGH 5//High frequency in GHz

// This is the base class for .dat --> .root converters.

class DatAnalyzer {
    public:
        DatAnalyzer(int numChannels, int numTimes, int numSamples, int res, float scale, int numFsamples = 0);
        ~DatAnalyzer();
        int getNumChannels() { return NUM_CHANNELS; }
        int getNumTimes() { return NUM_TIMES; }
        int getNumSamples() { return NUM_SAMPLES; }

        TString ParseCommandLine( int argc, char* argv[], TString opt );
        virtual void GetCommandLineArgs(int argc, char **argv);

        virtual void InitLoop();
        virtual void ResetVar(unsigned int n_ch);

        virtual void ResetAnalysisVariables();
        virtual int GetChannelsMeasurement() {
          std::cerr << "Please use a child class of DatAnalyzer" << std::endl;
          return 0;
        }
        virtual int GetChannelsMeasurement( int i ) {
          std::cerr << "Please use a child class of DatAnalyzer" << std::endl;
          return 0;
        }

        virtual unsigned int GetTimeIndex(unsigned int n_ch) { return n_ch; } // Return the index of the time associated with the channel n_ch
        virtual void Analyze();

        float GetPulseIntegral(float *a, float *t, unsigned int i_st, unsigned int i_stop); //returns charge in pC asssuming 50 Ohm termination
        unsigned int GetIdxClosest(float value, float* v, unsigned int i_st, int direction=+1);
        unsigned int GetIdxFirstCross(float value, float* v, unsigned int i_st, int direction=+1);
        void AnalyticalPolinomialSolver(unsigned int Np, float* in_x, float* in_y, unsigned int deg, float* &out_coeff, float* err = 0);
        float PolyEval(float x, float* coeff, unsigned int deg);
        float FrequencySpectrum(double freq, double tMin, double tMax, int ich, int t_index);
        float FrequencySpectrum(double freq, double tMin, double tMax, unsigned int n_samples, float* my_channel, float* my_time);
        int TimeOverThreshold(Interpolator *voltage, double tThresh, double tMin, double tMax, int ich, int t_index, float& time1, float& time2);


        void RunEventsLoop();

    protected:

        const unsigned int NUM_CHANNELS;
        const unsigned int NUM_TIMES;
        const unsigned int NUM_SAMPLES;
        const unsigned int NUM_F_SAMPLES;//Fourier samples
        const unsigned int DAC_RESOLUTION; // DAC resolution (2^[bit])
        const float DAC_SCALE; // [V] total scale of the DAC


        float scale_minimum = -500; // [mV] Voltage value corresponding to 0 DAC counts

        unsigned int N_warnings = 0;
        unsigned int N_warnings_to_print = 15;

        // Set by command line arguments or default
        Configuration* config = nullptr;

        TString input_file_path;
        TString output_file_path;
        unsigned long int N_evts = 0;
        unsigned int start_evt = 0;

        bool verbose = false;
        bool save_raw = false;
        bool save_meas = false;
        bool draw_debug_pulses = false;
        TString img_format = ".png";

        long int N_evt_expected = -1;

        // Reader variables
        FILE* bin_file = nullptr;

        // Analysis variables
        float* AUX_time;
        float* AUX_channel;
        float* AUX_channel_spectrum;

        float** time;
        float** channel;
        float** channel_spectrum;
        float* frequency;

        // Output tree vars
        unsigned int i_evt = 0;

        // Output root file
        TFile *file;
        TTree *tree;
        //Input root files (optional, now use by ETL simulation)
        TFile* file_in;
        TTree* tree_in;

        std::map<TString, float*> var;
        std::vector<TString> var_names = {
          "baseline",
          "baseline_RMS",
          "noise",
          "amp",
          "t_peak",
          "integral",
          "intfull",
          "risetime",
          "decaytime"
        };
};

#endif
