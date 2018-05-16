#include "SiPM_SimAnalyzer.hh"
#include "PulseShape.hh"

using namespace std;

void SiPM_SimAnalyzer::InitLoop(){
  DatAnalyzer::InitLoop();

/*
  char tmpTimeHeader[4];
  fread( tmpTimeHeader, sizeof(char), 4, bin_file );
  fread( tmpTimeHeader, sizeof(char), 4, bin_file );

  char tmpChar[2];
  short tmpNum;
  fread ( tmpChar, sizeof(char), 2, bin_file );
  fread ( &tmpNum, sizeof(short), 1, bin_file );
  cout << tmpChar[0] << tmpChar[1] << tmpNum << endl;

  char aux[4];
  do {
    fread( aux, sizeof(char), 4, bin_file);
    if( strncmp("C00", aux, 3) == 0 ) {
      unsigned int ch = aux[3] - '0' - 1;
      cout << aux[0] << aux[1] << aux[2] << aux[3] << " --> " << ch << endl;
      active_channels.push_back(ch);
      fread( &(event_time[ch][0]), sizeof(float), 1024, bin_file);
    }
  }
  while( strncmp("C00", aux, 3) == 0 );
  */
}

// Fill tc, raw, time and amplitude
int SiPM_SimAnalyzer::GetChannelsMeasurement() {
    // Initialize the output variables
    std::cout << "channel measurement" << std::endl;
    ResetAnalysisVariables();

    const double n_threshold = 10;
    const int n_experiments = 10;
    const double DCR = 0;
    const double Npe = 4000;
    const double scintillation_decay_constant = 40;
    const double scintillation_risetime = .0;
    const double single_photon_risetime_response = 0.5;
    const double single_photon_decaytime_response = 0.5;
    const double high_pass_filter_RC = 10000;

    std::cout << "number of experiments is: " << n_experiments << std::endl;
    std::cout << "Npe: " << Npe << std::endl;
    std::cout << "n_threshold: " << n_threshold << std::endl;
    std::cout << "scintillation decay constant: " << scintillation_decay_constant << " [ns]" << std::endl;
    std::cout << "scintillation_risetime: " << scintillation_risetime << " [ns]" << std::endl;
    std::cout << "single_photon_risetime_response: " << single_photon_risetime_response << " [ns]" << std::endl;
    std::cout << "single_photon_decaytime_response:" << single_photon_decaytime_response << " [ns]" << std::endl;
    std::cout << "high_pass_filter_RC: " << high_pass_filter_RC << std::endl;
    std::cout << "DCR: " << DCR << " [GHz]" << std::endl;


    PulseShape* ps;
    TGraph* total_pulse;
    TGraph* scintillation_pulse;
    TGraph* dark_noise;


    //double step = 0.01;
    double x_low  = -1e1;
    double x_high = 6e1;
    double step = (x_high-x_low)/SIM_SAMPLES;

    //const int npoints  = (x_high-x_low)/step;
    std::cout << "[INFO] number of points per pulse: " << SIM_SAMPLES << std::endl;
    std::cout << "[INFO] sampling rate is: " << step  << " ns" << std::endl;
    double x[SIM_SAMPLES];
    double y[SIM_SAMPLES], y_sc[SIM_SAMPLES], y_dc[SIM_SAMPLES];
    int i_evt;
    for ( int j = 0; j < n_experiments; j++ )
    {
      if ( j % 100 == 0 )std::cout << "experiment #" << j << std::endl;
      //reset variables and objects
      ps = new PulseShape("gauss");
      ps->SetNpe( Npe );
      ps->SetDCR( DCR );
      //ps->SetSinglePhotonResponse( single_pe_risetime );//units in ns
      ps->SetSinglePhotonRisetimeResponse( single_photon_risetime_response );
      ps->SetSinglePhotonDecaytimeResponse( single_photon_decaytime_response );
      ps->SetScintillationDecay( scintillation_decay_constant );//units in ns
      ps->SetHighPassFilterRC(high_pass_filter_RC);
      ps->NormalizeSinglePhotonResponse();
      //std::cout << ps->GetSinglePhotonResponseNormalization() << std::endl;
      ps->NormalizeSinglePhotonResponseHighPassFilter();
      //std::cout << ps->GetSinglePhotonResponseNormalization() << std::endl;
      for( int i = 0; i < SIM_SAMPLES; i++ ) y[i] = x[i] = 0.0;
      double y_max = 0;
      for( int i = 0; i < SIM_SAMPLES; i++ )
      {
        x[i] = x_low + double(i)*step;
        //if ( i % 1000 == 0 ) std::cout << "iteration #" << i << std::endl;
        //y[i]  = ps->Convolution(x[i], "Gauss", "RandomExp");
        y_sc[i]  = ps->ScintillationPulse(x[i]);
        y_dc[i]  = ps->DarkNoise(x[i], x_low, x_high);
        y[i]     = y_sc[i] + y_dc[i];
        channel[0][i] = y[i];
        time[0][i] = x[i];
        if( y[i] > y_max ) y_max = y[i];
      }
      delete ps;//release memory of pulseshape object.

      double t_stamp = -999;
      for( int i = 0; i < SIM_SAMPLES; i++ )
      {
        if ( y[i] > n_threshold )
        {
          t_stamp = (x[i]+x[i+1])/2.;
          break;
        }
      }
    }

    std::cout << "[INFO] out of simulation loop" << std::endl;
/*
    int event_number;
    fread(&event_number, sizeof(int), 1, bin_file);

    // Read date (YY/MM/DD/HH/mm/ss/ms).ms is milliseconds
    // Range center (RC) in mV, the span is always 1V;
    unsigned short event_date[7], range;
    fread(event_date, sizeof(short), 7, bin_file);
    fread(&range, sizeof(short), 1, bin_file);
    scale_minimum = range - 500.;

    // Read board id
    char tmpChar[2];
    short tmpNum;
    fread ( tmpChar, sizeof(char), 2, bin_file );
    fread ( &tmpNum, sizeof(short), 1, bin_file );

    // Read trigger
    short tcell;
    fread ( tmpChar, sizeof(char), 2, bin_file );
    fread ( &tcell, sizeof(short), 1, bin_file );

    bool event_end = false;
    vector<unsigned int> channel_detected;
    while(!event_end && !feof(bin_file)) {
      char channel_header[4];
      fread( channel_header, sizeof(char), 4, bin_file);
      if( strncmp("C00", channel_header, 3) != 0 ) {
        event_end = true;
      }
      else {
        unsigned int i_ch = channel_header[3] - '0' - 1;
        channel_detected.push_back(i_ch);

        int scaler;
        fread(&scaler, sizeof(int), 1, bin_file);

        // Get the channel measurement in DAC couts
        unsigned short raw[1024];
        fread( raw, sizeof(short), 1024, bin_file);

        // Assign to float and compute time
        time[i_ch][0] = 0;
        channel[i_ch][0] = raw[0];
        for(unsigned int i = 1; i<1024; i++) {
          channel[i_ch][i] = raw[i];
          time[i_ch][i] = time[i_ch][i-1] + event_time[i_ch][(tcell+i-1) % 1024];
        }

        if(channel_detected.size() > 0)
        {
          float dt = time[i_ch][(1024-tcell) % 1024];
          dt -= time[channel_detected[0]][(1024-tcell) % 1024];
          for(unsigned int i=0; i<1024; i++) {
            time[i_ch][i] -= dt;
          }
        }
      }
    }

    // for(uint j=0; j<2; j++) {
    //   cout << "-------- Channel " << j << endl;
    //   for(uint i=900; i < 1024; i++) {
    //     cout << time[j][i] << " - " << channel[j][i] << endl;
    //   }
    // }
*/
    return 0;
}

void SiPM_SimAnalyzer::Analyze(){
  DatAnalyzer::Analyze();
}
