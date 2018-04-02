#include "VMEAnalyzer.hh"

using namespace std;

void VMEAnalyzer::GetCommandLineArgs(int argc, char **argv){
  DatAnalyzer::GetCommandLineArgs(argc, argv);

  pixel_input_file_path = ParseCommandLine( argc, argv, "pixel_input_file" );
  if (pixel_input_file_path == ""){
    cout << "Pixel input file not provided" << endl;
  }
  else {
    cout << "Pixel input file: " << pixel_input_file_path.Data() << endl;
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
  cout << "Calibration file: " << calibration_file_path.Data() << "_bd1_group_[0-3]_[offset-dV].txt" << endl;
}

void VMEAnalyzer::LoadCalibration(){
  cout << "---------- Loading calibrations -------------" << endl;
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

void VMEAnalyzer::InitLoop(){
  DatAnalyzer::InitLoop();
  if(save_raw){
    tree->Branch("tc", tc, "tc[4]/s");
    cout << "   tc" << endl;
    tree->Branch("raw", raw, Form("raw[%d][%d]/s", NUM_CHANNELS, NUM_SAMPLES));
    cout << "   raw" << endl;
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
    cout << "   -->All pixel variables" << endl;
    idx_px_tree = min((unsigned long)start_evt, entries_px_tree-1);
    pixel_tree->GetEntry( idx_px_tree );
  }
}

int VMEAnalyzer::FixCorruption() {
  bool foundEventHeader = false;
  unsigned int long N_byte = 0;
  while (!foundEventHeader) {
    if (feof(bin_file)) return -1;

    //Look for the new header in steps of 8 bits
    N_byte++;
    char tmp = 0;
    fread( &tmp, sizeof(char), 1, bin_file);
    //find the magic word first
    if (tmp == 0x0C) {
      if (feof(bin_file)) return -1;
      fread( &tmp, sizeof(char), 1, bin_file);
      if (tmp == 0x36) {
        if (feof(bin_file)) return -1;
        unsigned short tmp2 = 0;
        fread( &tmp2, sizeof(unsigned short), 1, bin_file);
        if (tmp2 == 0xA000) {
          //now i'm good.
          foundEventHeader=true;
          cout << Form("Found a new event header after %ld bytes", N_byte) << endl;

	  //**********************************************************
	  //we need to increment the trigger counter an extra time 
	  //because we're skipping ahead to the next event
	  i_evt++;
	  //**********************************************************

          if(pixel_input_file_path != ""){
            cout << "Resetting the pixel tree" << endl;
            while (idx_px_tree < entries_px_tree && i_evt >= pixel_event->trigger) {
              pixel_tree->GetEntry(idx_px_tree);
              idx_px_tree++;
            }
          }
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
int VMEAnalyzer::GetChannelsMeasurement() {
    bool is_corrupted = false;
    ResetAnalysisVariables();
    // Initialize the output variables
    for(int j = 0; j < NUM_CHANNELS; j++) {
      if(j<NUM_TIMES){ tc[j] = 0; }
      for ( int i = 0; i < NUM_SAMPLES; i++ ) {
        raw[j][i] = 0;
      }
    }

    unsigned int event_header;

    // first header word
    fread( &event_header, sizeof(unsigned int), 1, bin_file);
    // second header word
    fread( &event_header, sizeof(unsigned int), 1, bin_file);
    unsigned int group_mask = event_header & 0x0f; // 4-bit channel group mask
    // third and fourth header words
    fread( &event_header, sizeof(unsigned int), 1, bin_file);
    fread( &event_header, sizeof(unsigned int), 1, bin_file);

    // check again for end of file
    if (feof(bin_file)) return -1;

    vector<unsigned int> active_groups;
    for(unsigned int k = 0; k<4; k++){
      if(group_mask & (0x1 << k)) active_groups.push_back(k);
    }

    //************************************
    // Loop over channel groups
    //************************************
    for(auto k : active_groups)
    {
      fread( &event_header, sizeof(unsigned int), 1, bin_file);
      tc[k] = (event_header >> 20) & 0xfff; // trigger counter bin

      // Check if all channels were active (if 8 channels active return 3072)
      int nsample = (event_header & 0xfff) / 3;
      if(nsample != 1024) {
        is_corrupted = true;
        cout << "[WARNING]: Corruption detected at event " << i_evt << ". Stopping the tree filling." << endl;
        cout << "[WARNING]: Corruption supposed by unexpected numeber of events. Events from header " << nsample << endl;
        break;
      }

      // Define time coordinate
      time[k][0] = 0.0;
      for( int j = 1; j < 1024; j++ ){
      	time[k][j] = float(j);
      	time[k][j] = float(tcal[k][(j-1+tc[k])%1024]) + time[k][j-1];
      }

      //************************************
      // Read sample info for group
      //************************************
      unsigned short samples[9][1024];
      unsigned int temp[3];
      for ( int j = 0; j < nsample; j++ ) {
      	fread( &temp, sizeof(unsigned int), 3, bin_file);
      	samples[0][j] =  temp[0] & 0xfff;
      	samples[1][j] = (temp[0] >> 12) & 0xfff;
      	samples[2][j] = (temp[0] >> 24) | ((temp[1] & 0xf) << 8);
      	samples[3][j] = (temp[1] >>  4) & 0xfff;
      	samples[4][j] = (temp[1] >> 16) & 0xfff;
      	samples[5][j] = (temp[1] >> 28) | ((temp[2] & 0xff) << 4);
      	samples[6][j] = (temp[2] >>  8) & 0xfff;
      	samples[7][j] =  temp[2] >> 20;
      }

      // Trigger channel
      for(int j = 0; j < nsample/8; j++){
      	fread( &temp, sizeof(unsigned int), 3, bin_file);
      	samples[8][j*8+0] =  temp[0] & 0xfff;
      	samples[8][j*8+1] = (temp[0] >> 12) & 0xfff;
      	samples[8][j*8+2] = (temp[0] >> 24) | ((temp[1] & 0xf) << 8);
      	samples[8][j*8+3] = (temp[1] >>  4) & 0xfff;
      	samples[8][j*8+4] = (temp[1] >> 16) & 0xfff;
      	samples[8][j*8+5] = (temp[1] >> 28) | ((temp[2] & 0xff) << 4);
      	samples[8][j*8+6] = (temp[2] >>  8) & 0xfff;
      	samples[8][j*8+7] =  temp[2] >> 20;
      }

      //************************************
      // Loop over channels 0-8
      //************************************
      for(int jj = 0; jj < 9; jj++) {
        int j_gl = k*9 + jj;
        if ( !config->hasChannel(j_gl) ) {
          for ( int j = 0; j < 1024; j++ ) {
            raw[j_gl][j] = 0;
            channel[j_gl][j] = 0;
          }
        }
        else{
          for ( int j = 0; j < 1024; j++ ) {
            raw[j_gl][j] = samples[jj][j];
            channel[j_gl][j] = (float)(samples[jj][j]) - off_mean[k][jj][(j+tc[k])%1024];
          }
        }
      }

      fread( &event_header, sizeof(unsigned int), 1, bin_file);
    } //loop over groups

    // Check if the following bytes corresponds to an event header. Important for skipping the event when the corruption happens during the last group;
    if (feof(bin_file)) return 0;
    fread( &event_header, sizeof(unsigned int), 1, bin_file);
    int magicWord1 = (event_header >> 28) & 0xf;
    int eventSize =  (event_header) & 0xfffffff;
    if (feof(bin_file)) return 0;
    fread( &event_header, sizeof(unsigned int), 1, bin_file);
    int boardID = (event_header >> 27) & 0x1f;
    int pattern = (event_header >> 8) & 0xffff;
    //reverse by 2 lines
    fseek(bin_file, -2*sizeof(unsigned int), SEEK_CUR);

    if (magicWord1 != 10 || pattern != 0 || eventSize != 13836) {
      is_corrupted = true;
      cout << "[WARNING] Following bitys not matching the expected header" << endl;
    }

    if(is_corrupted) {
      cout << "Found Data Corruption. Trying to skip to next event header...\n";
      return FixCorruption();
    }




    return 0;
}

void VMEAnalyzer::Analyze(){
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

    while (idx_px_tree < entries_px_tree && i_evt >= pixel_event->trigger) {
      pixel_tree->GetEntry(idx_px_tree);
      if (pixel_event->trigger == i_evt) {
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
      else if (i_evt > pixel_event->trigger) {
        cout << "[ERROR] Pixel tree not ordered" << endl;
        exit(0);
      }
    }

  }

  DatAnalyzer::Analyze();
}
