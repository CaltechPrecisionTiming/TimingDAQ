#include "KeysightAnalyzer.hh"

using namespace std;

void KeysightAnalyzer::InitLoop(){
  DatAnalyzer::InitLoop();

}

// Fill time and amplitude
int KeysightAnalyzer::GetChannelsMeasurement(int i_entry) {
    // Initialize the output variables
    ResetAnalysisVariables();

    float time_temp[Keysight_CHANNELS][Keysight_SAMPLES];
    float channel_temp[Keysight_CHANNELS][Keysight_SAMPLES];

    TBranch *time_branch = tree_in->GetBranch("time");
    TBranch *ch_branch = tree_in->GetBranch("channel");
    time_branch->SetAddress(time_temp);
    ch_branch->SetAddress(channel_temp);

    tree_in->GetEntry(i_entry);


    int i_ch = 0;
    for (unsigned int i_ch = 0; i_ch < Keysight_CHANNELS; i_ch++)
    {
      for(unsigned int i = 0; i<Keysight_SAMPLES; i++) {
          time[i_ch][i] = time_temp[i_ch][i]*1e9; //now time is in ns
          channel[i_ch][i] = channel_temp[i_ch][i];

      }
    }



    return 0;
}

void KeysightAnalyzer::Analyze(){
  DatAnalyzer::Analyze();
}
