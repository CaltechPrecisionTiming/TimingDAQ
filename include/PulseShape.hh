#ifndef PulseShape_HH
#define PulseShape_HH

#include <iostream>
#include <string>
//ROOT
#include <TF1.h>
#include <TROOT.h>
#include <TRandom3.h>
#include <TH1F.h>
#include <TMath.h>
#include <TGraph.h>
#include <TString.h>

class PulseShape
{
public:

  static bool _debug;
  static bool _info;
  static bool _warning;

  PulseShape();
  PulseShape( std::string function_name );
  PulseShape( std::string function_name, std::string integration_method );
  ~PulseShape();

  double Gauss( double x, double mean, double sigma, bool norm = true );
  double Exp( double x, double exponent );
  double RandomExp( double x, double exponent );

  double Convolution( double x, std::string function_name1, std::string function_name2 );
  double ScintillationPulse( double x );
  double DarkNoise( double x );//Dark Noise in the [x_low, x_high] region, units in ns
  double HighPassFilterResponse( double x );
  bool SetSinglePhotonResponse( std::string function_name );
  void SetNpe( int npe ){ Npe = npe;};
  void SetDCR( double dcr ){ DCR = dcr;};//in GHz
  void SetSinglePhotonResponse( double sigma ){ single_photon_response_sigma = sigma;};//units in ns
  void SetSinglePhotonRisetimeResponse( double risetime ){ single_photon_risetime_response = risetime;};//units in ns
  void SetSinglePhotonDecaytimeResponse( double decaytime ){ single_photon_decaytime_response = decaytime;};//units in ns
  void SetScintillationDecay( double tau_s ){ scintillation_decay_constant = tau_s;};//units in ns
  void SetHighPassFilterRC( double rc ){ high_pass_filter_RC = rc;};
  void NormalizeSinglePhotonResponse();
  void NormalizeSinglePhotonResponseHighPassFilter();
  double GetSinglePhotonResponseNormalization(){return single_photon_response_normalization;};
  void GenerateScintillationPhotonArrivalTime();
  void GenerateDarkNoisePhotonArrivalTime( double x_low, double x_high );


protected:
  std::string function_name;
  std::string integration_method;
  int Npe;// Number of photo-electrons (dE/dx*thickness*LightCollectionEfficiency*SiPM_PDE), about 4500 in LYSO
  double scintillation_decay_constant;//decay constant of the scintillator (LYSO is 40 ns )
  double scintillation_risetime;//rise time of the scintillator (LYSO is 60 ps)
  double single_photon_response_sigma;//sigma of the gaussian used to model the single photon response
  double single_photon_risetime_response;//tau1 in of A*t/tau1*exp(-t/tau1) - B*t/tau2*exp(-t/tau2) used to model the single photon response
  double single_photon_decaytime_response;//tau2 in of A*t/tau1*exp(-t/tau1) - B*t/tau2*exp(-t/tau2) used to model the single photon response
  double high_pass_filter_RC;//tau of the RC HighPassFilter (R*C) in ns
  double DCR;//dark count rate in GHz
  double single_photon_response_normalization;
  std::vector<double> t_sc_random;
  std::vector<double> t_dc_random;
  const double A = 3.0;
  const double B = 0.5;

};

#endif
