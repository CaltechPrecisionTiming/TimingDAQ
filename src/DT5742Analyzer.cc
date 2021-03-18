#include "DT5742Analyzer.hh"

using namespace std;

void DT5742Analyzer::GetCommandLineArgs(int argc, char **argv){
  DatAnalyzer::GetCommandLineArgs(argc, argv);

  pixel_input_file_path = ParseCommandLine( argc, argv, "pixel_input_file" );
  if (pixel_input_file_path == ""){
    if (verbose) { cout << "Pixel input file not provided" << endl; }
  }
  else {
    if (verbose) { cout << "Pixel input file: " << pixel_input_file_path.Data() << endl; }
    pixel_file = new TFile( pixel_input_file_path.Data(),"READ");
    if (!pixel_file) {std::cout << "[ERROR]: Pixel file not found" << std::endl; exit(0);}
    TString tree_name = pixel_file->GetListOfKeys()->At(0)->GetName(); //Only works if it the tree is the first key
    pixel_tree = (TTree*)pixel_file->Get(tree_name);
    if (!pixel_tree) {cout << "[ERROR]: Pixel Tree not found\n"; exit(0);}
    entries_px_tree = pixel_tree->GetEntries();
  }

  calibration_file_path = ParseCommandLine( argc, argv, "calibration_file" );
  if(calibration_file_path == ""){
    calibration_file_path = "calibration/v1740";
  }
  if (verbose) { cout << "Calibration file: " << calibration_file_path.Data() << "_bd1_group_[0-3]_[offset-dV].txt" << endl; }

  TString aux = ParseCommandLine( argc, argv, "Max_corruption" );
  if(aux != "") {
    Max_corruption = aux.Atoi();
  }
  if (verbose) { cout << "[INFO] Max corruption tollerated: " << Max_corruption << endl; }

  for(unsigned int i = 0; i<=Max_corruption; i++) manual_skip.push_back(-1);
  for(unsigned int i = 1; i<=Max_corruption; i++) {
    aux = ParseCommandLine( argc, argv, Form("NSkip%d", i) );
    if(aux == "") {
      manual_skip[i] = -1;
    }
    else {
      manual_skip[i] = aux.Atoi();
    }
  }

  //fseek (bin_file , 0 , SEEK_END);
  //lSize = ftell (bin_file);
  //printf("%d\n", lSize);
  //if (i_evt == 0 ) rewind (bin_file);
  // allocate memory to contain the whole file
  //buffer = (float*) malloc (sizeof(float)*lSize);
  //if (buffer == NULL) {fputs ("Memory error\n",stderr); exit (2);}
}

void DT5742Analyzer::LoadCalibration(){
  if (verbose) { cout << "---------- Loading calibrations -------------" << endl; }
  for(unsigned int ig = 0; ig < 4; ig++) {
    for(unsigned int is = 0; is < 4; is++) {
      tcal[ig][is] = 0;
      for(unsigned int ic = 0; ic < 9; ic++) off_mean[ig][ic][is] = 0;
    }
  }


  if(calibration_file_path == "ZEROS") return; //Uses defaul values (all 0)

  for( int i = 0; i < 4; i++ ){
      TString f_offset = calibration_file_path + Form("_bd1_group_%d_offset.txt",i);
      TString f_dV = f_offset;
      f_dV.ReplaceAll("_offset.txt", "_dV.txt");

      FILE* fp1 = fopen( f_offset.Data(), "r" );
      for( int k = 0; k < 1024; k++ ) {
          for( int j = 0; j < 9; j++ ){
              fscanf( fp1, "%lf ", &off_mean[i][j][k] );
          }
      }
      fclose(fp1);

      double tcal_dV[1024];
      double dV_sum = 0;
      fp1 = fopen( f_dV.Data(), "r" );
      for( int k = 0; k < 1024; k++) {
        double dummy, aux;
        fscanf( fp1, "%lf %lf %lf %lf %lf ", &dummy, &dummy, &dummy, &dummy, &aux );
        tcal_dV[k] = aux;
        dV_sum += aux;
      }
      fclose(fp1);

      for( int j = 0; j < 1024; j++) {
          tcal[i][j] = 200.0 * tcal_dV[j] / dV_sum;
      }
  }
}

void DT5742Analyzer::InitLoop(){
  DatAnalyzer::InitLoop();
  tree->Branch("triggerNumber", &triggerNumber, "triggerNumber/I");
  tree->Branch("corruption", &N_corr, "corruption/s");
  if (verbose) { cout << "   corruption" << endl; }

  if(save_raw){
    tree->Branch("tc", tc, "tc[4]/s");
    if (verbose) { cout << "   tc" << endl; }
    tree->Branch("raw", raw, Form("raw[%d][%d]/s", NUM_CHANNELS, NUM_SAMPLES));
    if (verbose) { cout << "   raw" << endl; }
  }

  if(pixel_input_file_path != ""){
    pixel_event = new FTBFPixelEvent;
    pixel_tree->SetBranchAddress("event", pixel_event);

    tree->Branch("xIntercept", &xIntercept, "xIntercept/F");
    tree->Branch("yIntercept", &yIntercept, "yIntercept/F");
    tree->Branch("xSlope", &xSlope, "xSlope/F");
    tree->Branch("ySlope", &ySlope, "ySlope/F");

    if(config->z_DUT.size()) {
      for(float zz : config->z_DUT) {
        x_DUT.push_back(0.);
        y_DUT.push_back(0.);
      }
      tree->Branch("x_dut", &(x_DUT[0]), Form("x_dut[%lu]/F", config->z_DUT.size()));
      tree->Branch("y_dut", &(y_DUT[0]), Form("y_dut[%lu]/F", config->z_DUT.size()));
    }
    tree->Branch("chi2", &chi2, "chi2/F");
    tree->Branch("ntracks", &ntracks, "ntracks/I");
    if (verbose) { cout << "   -->All pixel variables" << endl; }

    pixel_tree->GetEntry(0);
    while(pixel_event->trigger <  start_evt && idx_px_tree < entries_px_tree-1) {
      idx_px_tree++;
      pixel_tree->GetEntry( idx_px_tree );
    }
  }
}

int DT5742Analyzer::FixCorruption(int corruption_grp) {
  N_corr++;
  bool foundEventHeader = false;
  unsigned int long N_byte = 0;
  while (!foundEventHeader) {
    if (feof(bin_file)) return -1;

    //Look for the new header in steps of 8 bits
    N_byte++;
    char tmp = 0;
    fread( &tmp, sizeof(char), 1, bin_file);
    //find the magic word first
    if (tmp == (ref_event_size & 0xff)) {
      if (feof(bin_file)) return -1;
      fread( &tmp, sizeof(char), 1, bin_file);
      if (tmp == ((ref_event_size >> 8) & 0xff)) {
        if (feof(bin_file)) return -1;
        unsigned short tmp2 = 0;
        fread( &tmp2, sizeof(unsigned short), 1, bin_file);
        if ( tmp2 == (0xA000 + ((ref_event_size >> 16) & 0xfff)) ) {
          //now i'm good.
          foundEventHeader=true;
          cout << Form("Found a new event header after %ld bytes", N_byte) << endl;

      	  //**********************************************************
      	  //we need to increment the trigger counter an extra time
      	  //because we're skipping ahead to the next event
          if(manual_skip[N_corr] == -1) {
            if (corruption_grp<0) {
              i_evt++;
              cout << "Since corruption occured at the end of file, INCREMENTING THE i_evt" << endl;
            }
          }
          else {
            i_evt += manual_skip[N_corr];
            cout << "Manual skip set: " << manual_skip[N_corr] << " event skipped"<< endl;
          }


          if(pixel_input_file_path != ""){
            cout << "Resetting the pixel tree" << endl;
            while (idx_px_tree < entries_px_tree && i_evt >= pixel_event->trigger) {
              pixel_tree->GetEntry(idx_px_tree);
              idx_px_tree++;
            }
          }
      	  //**********************************************************

          // Reverse the two bytes read
          fseek(bin_file, -1*sizeof(unsigned int), SEEK_CUR);
          return 1;
        }
        else {
          fseek(bin_file, -1*sizeof(unsigned short), SEEK_CUR);
        }
      }
      else {
        fseek(bin_file, -1*sizeof(char), SEEK_CUR);
      }
    }
  }
  // Should not reach here. In case it does, stop the reading of the file.
  return -1;
}

// Fill tc, raw, time and amplitude
int DT5742Analyzer::GetChannelsMeasurement() {
    bool is_corrupted = false;
    int corruption_grp = -1;
    ResetAnalysisVariables();
    // Initialize the output variables
    for(int j = 0; j < NUM_CHANNELS; j++) {
      if(j<NUM_TIMES){ tc[j] = 0; }
      for ( int i = 0; i < NUM_SAMPLES; i++ ) {
        raw[j][i] = 0;
      }
    }

    //long lSize;
    //float* buffer;
    //size_t result;
    // obtain file size:

    //printf("%d\n", lSize);
    if ( i_evt == 0 )
    {
      fseek (bin_file , 0 , SEEK_END);
      lSize = ftell (bin_file);
      rewind (bin_file);
      // allocate memory to contain the whole file
      buffer = (float*) malloc (sizeof(float)*lSize);
      //std::cout << "[INFO]: n_events: " << nevents << std::endl;
    }

    if (buffer == NULL) {fputs ("Memory error\n",stderr); exit (2);}
    int  event_size = 73752;
    int nevents = lSize/event_size;
    N_evts = nevents;
    if ( i_evt == 0 ) std::cout << "[INFO]: n_events: " << nevents << std::endl;
    //std::cout << "nevents: " << nevents << std::endl;
    //*************************
    //Event Loop
    //*************************
    //i_evt = 0;
    //for( int ievent = 0; ievent < nevents; ievent++)
    //{
      //if ( ievent % 1000 == 0 ) std::cout << "[INFO]: processing event #" << ievent << std::endl;
      // copy the file into the buffer:
      result = fread (buffer,1,event_size,bin_file);
      //std::cout << result << " " << i_evt  << std::endl;
      if (result != event_size) {fputs ("Reading error\n",stderr); exit (3);}
      double x;
      x = *(buffer+6);
      x = *(buffer+6);


      for(int j = 0; j < DT5742_CHANNELS; j++)
      {
        for(int i = 0; i < DT5742_SAMPLES; i++ )
        {
	  raw[j][i] = buffer[i+6+j*DT5742_SAMPLES];
		  		//channel[j][i] = ( raw[j][i] - 2047. )/4096. ;//converting to volts [V]
          channel[j][i] = raw[j][i];
	  //std::cout << "i = " << i << " ; j = " << j << " ; raw[j][i] = " << raw[j][i] << " ; channel[j][i] = " << channel[j][i] << std::endl;
          //bin[i] = i;
          for (int k = 0; k < DT5742_TIMES; k++)
	  {
	    time[k][i] = i*1.0/DT5742_FREQ;

	  }

        }
      }
    //}
    return 0;
};

/*
**************************************************
Speficic analysis of DT5742, including telescope data
then calls main analyzer DatAnalyzer::Analyze()
**************************************************
*/
void DT5742Analyzer::Analyze(){
  if(pixel_input_file_path != ""){
    xIntercept = -999;
    yIntercept = -999;
    xSlope = -999;
    ySlope = -999;
    for(unsigned int i = 0; i < config->z_DUT.size(); i++) {
      x_DUT[i] = -999;
      y_DUT[i] = -999;
    }
    chi2 = -999.;
    ntracks = 0;

    while (idx_px_tree < entries_px_tree && i_evt >= (pixel_event->trigger+0)) {
      pixel_tree->GetEntry(idx_px_tree);
      if ((pixel_event->trigger+0) == i_evt) {
        if(ntracks == 0) {
          xIntercept = 1e-3*pixel_event->xIntercept; //um to mm
          yIntercept = 1e-3*pixel_event->yIntercept;
          xSlope = pixel_event->xSlope;
          ySlope = pixel_event->ySlope;
          for(unsigned int i = 0; i < config->z_DUT.size(); i++) {
            x_DUT[i] = xIntercept + xSlope*(config->z_DUT[i]);
            y_DUT[i] = yIntercept + ySlope*(config->z_DUT[i]);
          }
          chi2 = pixel_event->chi2;
        }
      	ntracks++;
        idx_px_tree++;
      }
      else if (i_evt > (pixel_event->trigger+0)) {
        cout << "[ERROR] Pixel tree not ordered" << endl;
        exit(0);
      }
    }

  }
  //calling main analyzer -- DatAnalyzer::Analyze() -- in DatAnalyzer.cc
  DatAnalyzer::Analyze();
}
