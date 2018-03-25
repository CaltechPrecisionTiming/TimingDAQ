#define DEFAULT_FOR_EMPTY_CH 0

#include "DatAnalyzer.hh"

using namespace std;

DatAnalyzer::DatAnalyzer(int numChannels, int numTimes, int numSamples, int res, float scale) :
        NUM_CHANNELS(numChannels), NUM_TIMES(numTimes), NUM_SAMPLES(numSamples),
        DAC_RESOLUTION(res), DAC_SCALE(scale),
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
    aux = "config/15may2017_new.config";
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

float DatAnalyzer::GetPulseIntegral(float *a, float *t, unsigned int i_st, unsigned int i_stop) //returns charge in pC asssuming 50 Ohm termination
{
  //Simpson's Rule for equaled space with Cartwright correction for unequaled space
  float integral = 0.;
  for (unsigned int i=i_st; i < i_stop-2 ; i+=2) {
    float aux = ( 2-(t[i+2]-t[i+1])/(t[i+1]-t[i]) ) * a[i];
    aux += (t[i+2]-t[i])*(t[i+2]-t[i])/((t[i+2]-t[i+1])*(t[i+1]-t[i])) * a[i+1];
    aux += ( 2-(t[i+1]-t[i])/(t[i+2]-t[i+1]) ) * a[i+2];

    integral += ( (t[i+2]-t[i]) / 6.0 ) * aux;
  }

  integral *= 1e-9 * 1e-3 * (1.0/50.0) * 1e12; //in units of pC, for 50Ohm termination
  return integral;
}

void DatAnalyzer::Analyze(){
  for(unsigned int i=0; i<NUM_CHANNELS; i++) {
    if ( !config->hasChannel(i) ) {
      ResetVar(i);
      continue;
    }

    TString name = Form("pulse_event%d_ch%d", i_evt, i);

    // Get the attenuation/amplification scale factor and convert ADC counts to mV
    float scale_factor = (1000 * DAC_SCALE / (float)DAC_RESOLUTION) * config->getChannelMultiplicationFactor(i);


    // ------- Get baseline ------
    float baseline = 0;
    unsigned int bl_st_idx = 5;
    unsigned int bl_lenght = 150;
    for(unsigned int j=bl_st_idx; j<(bl_st_idx+bl_lenght); j++) {
      baseline += channel[i][j];
    }
    baseline /= (float) bl_lenght;
    var["baseline"][i] = baseline;


    // ------------- Get minimum position, max amplitude and scale the signal
    unsigned int idx_min = 0;
    float amp = 0;
    for(unsigned int j=0; j<NUM_SAMPLES; j++) {
      channel[i][j] = scale_factor * (channel[i][j] - baseline);
      if(channel[i][j] < amp || j == 0) {
        idx_min = j;
        amp = channel[i][j];
      }
    }
    var["amp"][i] = amp;
    var["xmin"][i] = time[GetTimeIndex(i)][idx_min];

    // -------------- Integrate the pulse
    const int L_pk_shift = 20;
    const int R_pk_shift = 50;
    unsigned int l_bound = max(0, (int)idx_min-L_pk_shift);
    unsigned int r_bound = min(idx_min+R_pk_shift, NUM_SAMPLES-3);
    var["integral"][i] = GetPulseIntegral(channel[i], time[GetTimeIndex(i)], l_bound, r_bound);
    var["intfull"][i] = GetPulseIntegral(channel[i], time[GetTimeIndex(i)], 0, NUM_SAMPLES-3);



    // Draw plot of the pulse
    if(draw_debug_pulses) {
      // TODO: Set Y errors properly
      TGraphErrors* pulse = new TGraphErrors(NUM_SAMPLES, time[GetTimeIndex(i)], channel[i]);
      pulse->SetNameTitle("g_"+name, "g_"+name);

      TCanvas* c =  new TCanvas("c_"+name, "c_"+name);
      TLine* line = new TLine();
      // Draw pulse
      pulse->SetMarkerStyle(4);
      pulse->GetYaxis()->SetTitle("Amplitude [mV]");
      pulse->Draw("APE1");
      // Draw baseline
      line->SetLineWidth(1);
      line->SetLineColor(46);
      line->SetLineStyle(7);
      line->DrawLine(time[GetTimeIndex(i)][0], 0, time[GetTimeIndex(i)][NUM_SAMPLES-1], 0);
      line->SetLineStyle(1);
      line->DrawLine(time[GetTimeIndex(i)][bl_st_idx], 0, time[GetTimeIndex(i)][bl_st_idx+bl_lenght], 0);
      // Draw peak
      line->SetLineColor(8);
      line->SetLineStyle(4);
      line->DrawLine(time[GetTimeIndex(i)][0], var["amp"][i], var["xmin"][i], var["amp"][i]);
      line->DrawLine(var["xmin"][i], 0, var["xmin"][i], var["amp"][i]);
      // Draw integral area
      int left_bound = max(0, (int)idx_min-L_pk_shift);
      int right_bound = min(idx_min+R_pk_shift, NUM_SAMPLES-3);
      TGraph * integral_pulse = new TGraph(right_bound-left_bound, &(time[GetTimeIndex(i)][left_bound]), &(channel[i][left_bound]));
      integral_pulse->SetFillColor(40);
      integral_pulse->SetFillStyle(3144);
      integral_pulse->Draw("FC");
      TText* t_int = new TText(var["xmin"][i], var["amp"][i], Form("Int = %1.2e (%1.2e) pC", var["integral"][i], var["intfull"][i]));
      t_int->SetTextAlign(kHAlignLeft+kVAlignBottom);
      t_int->Draw();

      c->SetGrid();
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
