#define DEFAULT_FOR_EMPTY_CH 0

#include "DatAnalyzer.hh"

using namespace std;

DatAnalyzer::DatAnalyzer(int numChannels, int numTimes, int numSamples) :
        NUM_CHANNELS(numChannels), NUM_TIMES(numTimes), NUM_SAMPLES(numSamples),
        file(0), tree(0) {
    time = new float*[numTimes];
    channel = new float*[numChannels];

    for(unsigned int i=0; i<numChannels; i++) {
      channel[i] = new float[numSamples];
      if(i<numTimes) time[i] = new float[numSamples];
    }
    cout << "In DatAnalyzer constructor. Set NUM_CHANNELS to " << NUM_CHANNELS << flush;
    cout << ", NUM_TIMES to " << NUM_TIMES << endl;
    cout << " and NUM_SAMPLES to " << NUM_SAMPLES << endl;
}

DatAnalyzer::~DatAnalyzer() {
    cout << "In DatAnalyzer destructor" << endl;
    if (file) {
        file->Close();
    }
}

TString DatAnalyzer::ParseCommandLine( int argc, char* argv[], TString opt )
{
  TString out = "";
  for (int i = 1; i < argc && out==""; i++ ) {
    TString tmp( argv[i] );
    if ( tmp.Contains("--"+opt) ) {
      if(tmp.Contains("=")) {
        out = tmp(tmp.First("=")+1, tmp.Length());
      }
      else {
        out = "true";
      }
    }
  }
  return out;
}

void DatAnalyzer::GetCommandLineArgs(int argc, char **argv){

  input_file_path = ParseCommandLine( argc, argv, "input_file" );
  ifstream in_file(input_file_path.Data());
  if (!in_file || input_file_path == "" || !input_file_path.EndsWith(".dat")){
    cerr << "[ERROR]: please provide a valid input file. Use: --input_file=<your_input_file_path>.dat " << endl;
    exit(0);
  }
  else
  {
    cout << "Input file: " << input_file_path.Data() << endl;
  }

  output_file_path = ParseCommandLine( argc, argv, "output_file" );
  if ( output_file_path == "" ){
    output_file_path = input_file_path;
    output_file_path.ReplaceAll(".dat", ".root");
  }
  else if (!output_file_path.EndsWith(".root")) output_file_path += ".root";
  cout << "Output file: " << output_file_path.Data() << endl;

  // -------- Non compulsory command line arguments
  TString aux;

  aux = ParseCommandLine( argc, argv, "N_evts" );
  N_evts = aux.Atoi();
  cout << "Number of events: " << flush;
  if(N_evts == 0){ cout << "Not specified." << endl;}
  else{ cout << N_evts << endl;}

  aux = ParseCommandLine( argc, argv, "config" );
  if(aux == ""){
    aux = "config/15may2017.config";
  }
  cout << "Config file: " << aux.Data() << endl;
  config = new Config(aux.Data());
  if ( !config->hasChannels() || !config->isValid() ) {
    cerr << "\nFailed to load channel information from config " << aux.Data() << endl;
    exit(0);
  }

  aux = ParseCommandLine( argc, argv, "save_raw" );
  aux.ToLower();
  if(aux == "true") save_raw =  true;

  aux = ParseCommandLine( argc, argv, "save_meas" );
  aux.ToLower();
  if(aux == "true") save_meas =  true;

  aux = ParseCommandLine( argc, argv, "draw_debug_pulses" );
  aux.ToLower();
  if(aux == "true") {
    cout << "[INFO]: Showing debug pulses" << endl;
    draw_debug_pulses =  true;
  }
}

void DatAnalyzer::InitTree() {
    cout << "In DatAnalyzer::InitTree" << endl;
    file = new TFile(output_file_path.Data(), "RECREATE");
    tree = new TTree("pulse", "Digitized waveforms");

    tree->Branch("i_evt", &i_evt, "i_evt/i");

    if(save_meas){
      tree->Branch("channel", &(channel[0][0]), Form("channel[%d][%d]/F", NUM_CHANNELS, NUM_SAMPLES));
      tree->Branch("time", &(time[0][0]), Form("time[%d][%d]/F", NUM_TIMES, NUM_SAMPLES));
    }

    for(auto n : var_names){
      var[n] = new float[NUM_CHANNELS];
      tree->Branch(n, &(var[n]), n+Form("[%d]/F", NUM_CHANNELS));
    }
}

void DatAnalyzer::ResetVar(unsigned int n_ch) {
  for(auto n: var_names) {
    var[n][n_ch] = DEFAULT_FOR_EMPTY_CH;
  }
}

void DatAnalyzer::ResetAnalysisVariables() {
  for(unsigned int i=0; i<NUM_CHANNELS; i++) {
    for(unsigned int j=0; j<NUM_SAMPLES; j++) {
      channel[i][j] = 0;
      if(i < NUM_TIMES) time[i][j] = 0;
    }
  }
}

void DatAnalyzer::Analyze(){
  cout << "Should analyze" << endl;

  for(unsigned int i=0; i<NUM_CHANNELS; i++) {
    if ( !config->hasChannel(i) ) {
      ResetVar(i);
      continue;
    }

    TString name = Form("pulse_event%d_ch%d", i_evt, i);

    TGraphErrors* pulse = new TGraphErrors(NUM_SAMPLES, time[GetTimeIndex(i)], channel[i]);
    pulse->SetNameTitle("g_"+name, "g_"+name);

    if(draw_debug_pulses) {
      TCanvas* c =  new TCanvas("c_"+name, "c_"+name);
      pulse->Draw("APE1*");
      c->SaveAs("~/Desktop/debug/"+name+".png");
    }
  }
}

void DatAnalyzer::RunEventsLoop() {
    cout << "In DatAnalyzer::RunEventsLoop" << endl;
    InitTree();

    bin_file = fopen( input_file_path.Data(), "r" );
    unsigned int N_written_evts = 0;
    for( i_evt = 0; !feof(bin_file) && (N_evts==0 || i_evt<N_evts); i_evt++){
        int out = GetChannelsMeasurement();
        if(out == -1) break;

        Analyze();

        N_written_evts++;
        tree->Fill();
    }

    fclose(bin_file);
    cout << "\nProcessed total of " << N_written_evts << " events\n";

    file->Write();
    file->Close();
}
