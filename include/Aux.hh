#ifndef Aux_HH
#define Aux_HH

#include <TGraphErrors.h>
#include <TH1F.h>
#include <TF1.h>
#include <TCanvas.h>
#include <string>

static const int nPoints = 24;
static float outputAmplitude[nPoints] = { 11.7, 13.5, 16.7, 20.7, 24.6, 33.6, 47.4, 59, 73, 87,
    101, 116, 131, 144, 163, 197, 235, 306, 391, 466, 544, 669, 831, 937 };

//correcting the extrapolated points by measured / extrapolated ratio
static float amplificationFactor[nPoints] = { 27.9/1.37*10 , 26.8/1.37*10, 31.6/1.37*10,
    33.9/1.37*10, 37.3/1.37*10, 40.2/1.37*10, 43.5/1.37*10, 43.1/1.37*10, 44.1/1.37*10,
    45.5/1.37*10, 34*10, 35.6*10, 37*10, 40*10, 42*10, 45*10, 48*10, 53*10, 58*10, 61*10,
    63*10, 66*10, 68*10, 66*10 };

double GetAmplificationFactor ( double measuredAmplitude );
TGraphErrors* WeierstrassTransform( short* channel, float* time, TString pulseName, double sigma = 1.0, bool makePlot = false );
void WeierstrassTransform( short* channel, double* channelFilter, float* time, TString pulseName, double sigma );
TGraphErrors* GetTGraph( double* channel, float* time );
TGraphErrors GetTGraph( short* channel, float* time );
double GetGaussTime( TGraphErrors* pulse );
void HighPassFilter( short* channel, double* filteredCurrent, float* time, double R = -1.0, double C = -1.0 );
void NotchFilter( short* channel, double* filteredCurrent, float* time, double R = -1.0, double C = -1.0, double L = -1.0 );
int FindMin( int n, short *a);
int FindRealMin( int n, short *a);
int FindMinAbsolute( int n, double *a);
int FindMinAbsolute( int n, short *a);
int FindMinAbsolute( int n, short *a, int leftBoundary, int rightBoundary);
int FindMinFirstPeakAboveNoise( int n, short *a);
float GausFit_MeanTime(TGraphErrors * pulse, const float index_first, const float index_last);
float RisingEdgeFitTime(TGraphErrors * pulse, const float index_min, const float constantFraction, TString fname, bool makePlot = false );
void RisingEdgeFitTime(TGraphErrors * pulse, const float index_min, float* tstamp, int event, TString fname, bool makePlot = false);
void RisingEdgeFitTime(TGraphErrors * pulse, const float index_min, const float fitLowEdge, const float fitHighEdge,
		       float* tstamp, int event, TString fname, bool makePlot = false );
void TailFitTime(TGraphErrors * pulse, const float index_min, float* tstamp, int event, TString fname, bool makePlot = false );
float SigmoidTimeFit(TGraphErrors * pulse, const float index_min, int event, TString fname, bool makePlot = false );
float FullFitScint( TGraphErrors * pulse, const float index_min, int event, TString fname, bool makePlot = false );
float GausFit_MeanTime(TGraphErrors* pulse, const float index_first, const float index_last, TString fname);
float GetBaseline( int peak, short *a );
float GetBaseline(TGraphErrors * pulse, int i_low, int i_high, TString fname );
float GetPulseIntegral(int peak, short *a, std::string option = "", int nsamplesL=20, int nsamplesR=40);
float GetPulseIntegral(int peak, int nsamplesL, int nsamplesR, short *a, float *t);
float ConstantThresholdTime(TGraphErrors * pulse, const float threshold);
bool isRinging( int peak, short *a );
std::string ParseCommandLine( int argc, char* argv[], std::string opt );

#endif
