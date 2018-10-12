#include "ETL_ASIC_Analyzer.hh"

using namespace std;

void ETL_ASIC_Analyzer::InitLoop(){
  DatAnalyzer::InitLoop();

  //float mych[5000];
  //float mytime[5000];
  //tree_in->SetBranchAddress("channel", mych);
  //tree_in->SetBranchAddress("time", mytime);
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
int ETL_ASIC_Analyzer::GetChannelsMeasurement( int i ) {
    // Initialize the output variables
    ResetAnalysisVariables();
    unsigned int i_evt;
    float mych[ETL_ASIC_SAMPLES];
    float mytime[ETL_ASIC_SAMPLES];
    //tree_in->SetBranchStatus("shapednoise",0);
    //tree_in->SetBranchStatus("noise",0);
    tree_in->SetBranchAddress("i_evt", &i_evt);
    tree_in->SetBranchAddress("channel", &mych[0]);
    tree_in->SetBranchAddress("time", &mytime[0]);
    tree_in->GetEntry(i);
    //std::cout << "========= i_evt: " << i_evt  << "==========" << std::endl;
    //filling channel information
    for ( int j = 0; j < ETL_ASIC_CHANNELS; j++ )
    {
      for ( int k = 0; k < ETL_ASIC_SAMPLES; k++)
      {
        channel[j][k] = mych[k];
        //std::cout << "channel[" << k << "]: " << channel[j][k] << std::endl;
      }
    }

    //filling time information
    for ( int j = 0; j < ETL_ASIC_TIMES; j++ )
    {
      for ( int k = 0; k < ETL_ASIC_SAMPLES; k++)
      {
        time[j][k] = mytime[k];
        //std::cout << "channel: " << time[j][k] << std::endl;
      }
    }


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

void ETL_ASIC_Analyzer::Analyze(){
  DatAnalyzer::Analyze();
}
