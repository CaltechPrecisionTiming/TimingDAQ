#include "NetScopeStandaloneAnalyzer.hh"
#define BUFSIZE 8192

using namespace std;

void NetScopeStandaloneAnalyzer::GetCommandLineArgs(int argc, char **argv){
  DatAnalyzer::GetCommandLineArgs(argc, argv);
}


void NetScopeStandaloneAnalyzer::InitLoop(){
  DatAnalyzer::InitLoop();

  tree_in->SetBranchAddress("i_evt", &i_evt);
  tree_in->SetBranchAddress("channel", &(channel[0][0]));
  tree_in->SetBranchAddress("time", &(time[0][0]));
}

// Fill tc, raw, time and amplitude
int NetScopeStandaloneAnalyzer::GetChannelsMeasurement(int i_aux) {
  ResetAnalysisVariables();

  tree_in->GetEntry(i_aux);

  return 0;
}
