#include "SiPM_SimAnalyzer.hh"

using namespace std;

void SiPM_SimAnalyzer::InitLoop(){
  DatAnalyzer::InitLoop();

  const double n_threshold = config->n_threshold;
  //const int n_experiments = 1;
  const double DCR = config->DCR;
  const double scintillation_decay_constant = config->scintillation_decay_constant;
  const double scintillation_risetime = config->scintillation_risetime;
  const double single_photon_risetime_response = config->single_photon_risetime_response;
  const double single_photon_decaytime_response = config->single_photon_decaytime_response;
  const double high_pass_filter_RC = config->high_pass_filter_RC;


  std::cout << "Npe: " << config->Npe << std::endl;
  std::cout << "n_threshold: " << n_threshold << std::endl;
  std::cout << "scintillation decay constant: " << scintillation_decay_constant << " [ns]" << std::endl;
  std::cout << "scintillation_risetime: " << scintillation_risetime << " [ns]" << std::endl;
  std::cout << "single_photon_risetime_response: " << single_photon_risetime_response << " [ns]" << std::endl;
  std::cout << "single_photon_decaytime_response:" << single_photon_decaytime_response << " [ns]" << std::endl;
  std::cout << "high_pass_filter_RC: " << high_pass_filter_RC << std::endl;
  std::cout << "DCR: " << DCR << " [GHz]" << std::endl;

  ps = new PulseShape("gauss");
  ps->SetSinglePhotonRisetimeResponse( single_photon_risetime_response );
  ps->SetSinglePhotonDecaytimeResponse( single_photon_decaytime_response );
  ps->SetScintillationDecay( scintillation_decay_constant );//units in ns
  ps->SetHighPassFilterRC(high_pass_filter_RC);
  ps->NormalizeSinglePhotonResponse();
  ps->NormalizeSinglePhotonResponseHighPassFilter();
  ps->SetDCR( DCR );
}

// Fill tc, raw, time and amplitude
int SiPM_SimAnalyzer::GetChannelsMeasurement() {
    // Initialize the output variables
    if (i_evt % 5 == 0) {cout << i_evt << endl;}
    ResetAnalysisVariables();

    double x_low  = -10.;
    double x_high = 200.;
    double step = (x_high-x_low)/SIM_SAMPLES;

    ps->SetNpe( gRandom->Poisson(config->Npe) );

    ps->GenerateScintillationPhotonArrivalTime();
    ps->GenerateDarkNoisePhotonArrivalTime( x_low, x_high );

    for( int i = 0; i < SIM_SAMPLES; i++ )
    {
      time[0][i] = x_low + i * step;
      channel[0][i] = ps->ScintillationPulse(time[0][i]) + ps->DarkNoise(time[0][i]);
    }

    return 0;
}

void SiPM_SimAnalyzer::Analyze(){
  DatAnalyzer::Analyze();
}
