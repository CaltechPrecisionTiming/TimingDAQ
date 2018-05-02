#include "DRSclAnalyzer.hh"

using namespace std;

void DRSclAnalyzer::InitLoop(){
  DatAnalyzer::InitLoop();

  char board_id[6];
  fread( board_id, sizeof(char), 6, bin_file );
  for(uint i = 0; i<6; i++){cout << board_id[i] << flush;};
  cout << endl;

  char ch_mask[6];
  fread( ch_mask, sizeof(char), 6, bin_file );
  for(unsigned int i = 0; i<4; i++) {
    if (ch_mask[5] & (unsigned int)(pow(2,i))) {
      active_channels.push_back(i);
      cout << "Channel " << i+1 << " expected" << endl;
    }
  }

  fread ( board_id, sizeof(char), 2, bin_file );
  cout << board_id[0] << board_id[1] << ": " << flush;
  fread ( &N_evt_expected, sizeof(unsigned int), 1, bin_file );
  cout << N_evt_expected << endl;
  N_evt_expected = -1;
}

// Fill tc, raw, time and amplitude
int DRSclAnalyzer::GetChannelsMeasurement() {
    // Initialize the output variables
    ResetAnalysisVariables();

    char tmp[3];
    unsigned int event_number;
    fread(tmp, sizeof(char), 3, bin_file);
    fread(&event_number, sizeof(unsigned), 1, bin_file);

    for( auto i : active_channels) {
      fread(time[i], sizeof(float), 1024, bin_file);
      fread(channel[i], sizeof(float), 1024, bin_file);
    }

    return 0;
}

void DRSclAnalyzer::Analyze(){
  DatAnalyzer::Analyze();
}
