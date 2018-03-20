// C++ includes
#include <fstream>
#include <string>
#include <iostream>

// ROOT includes
#include <TROOT.h>
#include <TStyle.h>
#include <TFile.h>
#include <TTree.h>
#include <TGraphErrors.h>
#include <TCanvas.h>

//LOCAL INCLUDES
#include "Aux.hh"
#include "Config.hh"
#include "timingAlgorithm.h"

using namespace std;

TStyle* style;

int graphic_init();

int main(int argc, char **argv) {
  gROOT->SetBatch();

  FILE* fp1;
  char stitle[200]; 
  int dummy;

  //**************************************
  // Parse command line arguments
  //**************************************

  int numRequiredArgs = 3;
  if (argc - 1 < numRequiredArgs) {
    std::cerr << "Usage: dat2root in_file.dat num_events" << std::endl;
    return -1;
  }
  std::cout << "\n=== Beginning program ===\n" << std::endl;

  std::string inputFilename = argv[1];
  std::string outputFilename = argv[2];
  std::cout << "Input file: " << inputFilename << std::endl;
  std::cout << "Output file: " << outputFilename << std::endl;

  // Check if has valid input file, otherwise exit with error
  ifstream ifile(inputFilename);
  if (!ifile) {
    printf("!USAGE! Input file does not exist. Please enter valid file name"); 
    exit(0);}

  // Check if input file is root type or dat type
  // datfile: true is dat, false is root
  bool DATTYPE;

  std::string filetype = inputFilename.substr(inputFilename.find_last_of(".") + 1);

  if (filetype  == "dat") {
    DATTYPE = true;
  }
  else if (filetype == "root") {
    DATTYPE = false;
  }
  else {
    printf("!USAGE! Invalid input file type");
    exit(0);
  }
  
  
  int nEvents = atoi(argv[3]);
  std::cout << "Will process " << nEvents << " events" << std::endl;

  // Board number is fixed at 1 for now because we only have one board
  std::string boardNumber = "1";
  std::cout << "Will use calibration files for board number " << boardNumber << "\n";

  bool saveRaw = false;
  std::string _saveRaw = ParseCommandLine( argc, argv, "--saveRaw" );
  if ( _saveRaw == "yes" ) {
    saveRaw = true;
    std::cout << "Will save raw pulses\n";
  }

  bool drawDebugPulses = false;
  std::string _drawDebugPulses = ParseCommandLine( argc, argv, "--debug" );
  if ( _drawDebugPulses == "yes" ) {
    drawDebugPulses = true;
    std::cout << "draw: " << drawDebugPulses << std::endl;
  }

  std::string configName = "config/15may2017.config";
  //std::string configName = "alignmentTestConfig.config";
  std::string _configName = ParseCommandLine( argc, argv, "--config" );
  if ( _configName != "" ) {
    configName = _configName;
  }

  //	bool doFilter = false;
  //	std::string _doFilter = ParseCommandLine( argc, argv, "--doFilter" );
  //	if ( _doFilter == "yes" ) {
  //		saveRaw = true;
  //		doFilter = true;
  //		std::cout << "Will apply Weierstrass transform (gaussian filter) to input pulses\n";
  //	}


  std::cout << "\n=== Parsing configuration file " << configName << " ===\n" << std::endl;
  Config config(configName);
  if ( !config.hasChannels() || !config.isValid() ) {
    std::cerr << "\nFailed to load channel information from config " << configName << std::endl;
    return -1;
  }

  //// ONLY DO IF DAT FILETYPE

  double tcal_dV[4][1024];
  double tcal[4][1024];
  double off_mean[4][9][1024];

  if (DATTYPE) {

    //**************************************
    // Load Voltage Calibration
    //**************************************

    std::cout << "\n=== Loading voltage calibration ===\n" << std::endl;

    for( int i = 0; i < 4; i++ ){
      sprintf( stitle, "v1740_bd%s_group_%d_offset.txt", boardNumber.c_str(), i );
      fp1 = fopen( stitle, "r" );
      printf("Loading offset data from %s\n", stitle);

      for( int k = 0; k < 1024; k++ ) {     
	for( int j = 0; j < 9; j++ ){      
	  dummy = fscanf( fp1, "%lf ", &off_mean[i][j][k] ); 
	}
      }
      fclose(fp1);
    }

    //**************************************
    // Load Time Calibration
    //**************************************

    std::cout << "\n=== Loading time calibration ===\n" << std::endl;
    double fdummy;

    for( int i = 0; i < 4; i++ ) {
      sprintf( stitle, "v1740_bd%s_group_%d_dV.txt", boardNumber.c_str(), i );
      fp1 = fopen( stitle, "r" );
      printf("Loading dV data from %s\n", stitle);

      for( int k = 0; k < 1024; k++)      
	dummy = fscanf( fp1, "%lf %lf %lf %lf %lf ", 
			&fdummy, &fdummy, &fdummy, &fdummy, &tcal_dV[i][k] ); 
      fclose(fp1);
    }
    double dV_sum[4] = {0, 0, 0, 0};
    for( int i = 0; i < 4; i++ ) {
      for( int j = 0; j < 1024; j++ )
	dV_sum[i] += tcal_dV[i][j];
    }


    for( int i = 0; i < 4; i++) {
      for( int j = 0; j < 1024; j++) {
	tcal[i][j] = tcal_dV[i][j] / dV_sum[i] * 200.0;
      }
    }

  }

  //// DO FOR ALL FILE TYPES

  //**************************************
  // Define output
  //**************************************

  TFile* file = new TFile( outputFilename.c_str(), "RECREATE", "CAEN V1742");
  TTree* tree = new TTree("pulse", "Digitized waveforms");

  int event;
  UShort_t tc[4]; // trigger counter bin
	
  // want groups so set everything to -1 for init
  float time[4][1024]; // calibrated time
	
  //memset(time, -1, sizeof(time[0][0]) * 4 * 1024);
	
  for (int i =0; i < 4; i++) {
    for (int j=0; j < 1024; j++) {
      //if (time[i][j] != -1)
      //std::cout << "NOT CORRECT" << std::endl;
      time[i][j] = -999.0;
    }
  }
  
  //std::cout << "FINISHED TESTING" << std::endl;


  short raw[36][1024]; // ADC counts
  short channel[36][1024]; // calibrated input (in V)
  double channelFilter[36][1024]; // calibrated input (in V)
  float xmin[36]; // location of peak
  float xminRestricted[36]; //location of peak restricted near to channel 0 reference
  float base[36]; // baseline voltage
  float amp[36]; // pulse amplitude
  float ampRestricted[36]; // pulse amplitude within a restricted window near channel 0 reference
  float integral[36]; // integral in a window
  float integralFull[36]; // integral over all bins
  float gauspeak[36]; // time extracted with gaussian fit
  float sigmoidTime[36];//time extracted with sigmoid fit
  float linearTime0[36]; // constant fraction fit coordinates
  float linearTime15[36];
  float linearTime30[36];
  float linearTime45[36];
  float linearTime60[36];       
  float fallingTime[36]; // falling exponential timestamp
  float exactTimeCFD[36]; //algorithm from Nicola Minafra
	
  float risetime[36]; 
  float constantThresholdTime[36];
  bool _isRinging[36];

  float xIntercept;
  float yIntercept;
  float xSlope;
  float ySlope;
  float x1;
  float y1;
  float x2;
  float y2;
  
  tree->Branch("event", &event, "event/I");
  tree->Branch("tc", tc, "tc[4]/s");
  if (saveRaw) {
    tree->Branch("raw", raw, "raw[36][1024]/S");   
  }
  tree->Branch("channel", channel, "channel[36][1024]/S");
  tree->Branch("channelFilter", channelFilter, "channelFilter[36][1024]/D");
  tree->Branch("time", time, "time[4][1024]/F");
  tree->Branch("xmin", xmin, "xmin[36]/F");
  tree->Branch("xminRestricted", xminRestricted, "xminRestricted[36]/F");
  tree->Branch("amp", amp, "amp[36]/F");
  tree->Branch("ampRestricted", ampRestricted, "ampRestricted[36]/F");
  tree->Branch("base", base, "base[36]/F");
  tree->Branch("integral", integral, "integral[36]/F");
  tree->Branch("intfull", integralFull, "intfull[36]/F");
  tree->Branch("gauspeak", gauspeak, "gauspeak[36]/F");
  tree->Branch("sigmoidTime", sigmoidTime, "sigmoidTime[36]/F");
  tree->Branch("linearTime0", linearTime0, "linearTime0[36]/F");
  tree->Branch("linearTime15", linearTime15, "linearTime15[36]/F");
  tree->Branch("linearTime30", linearTime30, "linearTime30[36]/F");
  tree->Branch("linearTime45", linearTime45, "linearTime45[36]/F");
  tree->Branch("linearTime60", linearTime60, "linearTime60[36]/F");
  tree->Branch("fallingTime", fallingTime, "fallingTime[36]/F");
  tree->Branch("exactTimeCFD", exactTimeCFD, "exactTimeCFD[36]/F");
  tree->Branch("risetime", risetime, "risetime[36]/F");
  tree->Branch("constantThresholdTime", constantThresholdTime, "constantThresholdTime[36]/F");
  tree->Branch("isRinging", _isRinging, "isRinging[36]/O");  
  tree->Branch("xIntercept", &xIntercept, "xIntercept/F");
  tree->Branch("yIntercept", &yIntercept, "yIntercept/F");
  tree->Branch("xSlope", &xSlope, "xSlope/F");
  tree->Branch("ySlope", &ySlope, "ySlope/F");
  tree->Branch("x1", &x1, "x1/F");
  tree->Branch("y1", &y1, "y1/F");
  tree->Branch("x2", &x2, "x2/F");
  tree->Branch("y2", &y2, "y2/F");
  
  // temp variables for data input
  uint   event_header;
  uint   temp[3];
  ushort samples[9][1024];

  //*************************
  // Open Input File
  //*************************

  FILE *fpin = 0;
  TFile *rootInput = 0;
  TTree *rootInputTree = 0;
  

  int nGoodEvents = 0;

  // Set according to if dat type or root type
  if (DATTYPE) {
    fpin= fopen( inputFilename.c_str(), "r" );
  }
  else {
    rootInput = new TFile( inputFilename.c_str() );
    rootInputTree = (TTree *)rootInput->Get("pulse");
		
    rootInputTree->SetBranchAddress("event", &event);    
    rootInputTree->SetBranchAddress("time", time);    
    rootInputTree->SetBranchAddress("channel", channel);
    rootInputTree->SetBranchAddress("base", base);
    rootInputTree->SetBranchAddress("xIntercept", &xIntercept);
    rootInputTree->SetBranchAddress("yIntercept", &yIntercept);
    rootInputTree->SetBranchAddress("xSlope", &xSlope);
    rootInputTree->SetBranchAddress("ySlope", &ySlope);
    rootInputTree->SetBranchAddress("x1", &x1);
    rootInputTree->SetBranchAddress("y1", &y1);
    rootInputTree->SetBranchAddress("x2", &x2);
    rootInputTree->SetBranchAddress("y2", &y2);
    
    //copy over other stuff that wont get overwritten
    rootInputTree->SetBranchAddress("tc", tc);
		
    if (saveRaw) {
      rootInputTree->SetBranchAddress("raw", raw); }
  }


  //// Only for dat data type:


  //*************************
  //Event Loop
  //*************************
  int maxEvent = min( int(rootInputTree->GetEntries()), nEvents);
  if (nEvents < 0) maxEvent = rootInputTree->GetEntries();
  std::cout << "\nMaxEvents = " << maxEvent << "\n";
  
  std::cout << "\n=== Processing input data ===\n" << std::endl;
  for( int iEvent = 0; iEvent < maxEvent; iEvent++){ 
    
    //std::cout << "1" << std::endl;
    
    
    if ( iEvent % 100 == 0 ) {
      std::cout << "Event " << iEvent << " of " << maxEvent << std::endl; }

    if (DATTYPE) event = nGoodEvents; // for output tree
    
    int activeGroupsN = 0;
    int realGroup[4] = {-1, -1, -1, -1};
    int totalIndex;
    // SET THINGS FOR DATTYPE
    
    //std::cout << "2" << std::endl;
    
    if (DATTYPE) {
      
      // first header word
      dummy = fread( &event_header, sizeof(uint), 1, fpin);
      // second header word
      dummy = fread( &event_header, sizeof(uint), 1, fpin);  
      uint grM     = event_header & 0x0f; // 4-bit channel group mask
      // third and fourth header words
      dummy = fread( &event_header, sizeof(uint), 1, fpin);  
      dummy = fread( &event_header, sizeof(uint), 1, fpin);  

      // check for end of file
      if (feof(fpin)) {
	std::cout << "breaking" << std::endl;
	break; }

      //*************************
      // Parse group mask into channels
      //*************************
      
      //std::cout << "3" << std::endl;
      
      bool _isGR_On[4];
      _isGR_On[0] = (grM & 0x01);
      _isGR_On[1] = (grM & 0x02);
      _isGR_On[2] = (grM & 0x04);
      _isGR_On[3] = (grM & 0x08);

      //std::cout << "4" << std::endl;
		  
      for ( int l = 0; l < 4; l++ ) {
	if ( _isGR_On[l] ) 
	  {
	    realGroup[activeGroupsN] = l; 
	    activeGroupsN++;
	  }
      }
		  
      //std::cout << "DAT TYPE ACTIVE GROUPS: " << activeGroupsN << std::endl;
    }
    
    // SET THINGS FOR ROOT TYPE
    
    //std::cout << "5" << std::endl;
    
    if (!DATTYPE) {
      
      rootInputTree->GetEntry(iEvent);
      
      //std::cout << "6" << std::endl;
      
      // get groups from time
      
      // assuming time can never be negative
      
      for (int i = 0; i < 4; i++) {
        if (time[i][1] > 0) {
          realGroup[activeGroupsN] = i;
          activeGroupsN++; 
        }
        //for (int j = 0; j < 1024; j++) {
	//if (time[i][j] > 0) {
	//std::cout << "TIME: " << i << " , " << j << std::endl; } }
      }
      
      //std::cout << "ROOT ACTIVE GROUPS: " << activeGroupsN << std::endl;
    } 
    
    //std::cout << "7" << std::endl;
    
    //************************************
    // Loop over channel groups
    //************************************

    for ( int group = 0; group < activeGroupsN; group++ ) {
      ushort tcn; 
		  
      //std::cout << "8" << std::endl;
		  
      if (DATTYPE) {
		    
	// Read group header
	dummy = fread( &event_header, sizeof(uint), 1, fpin);  
	tcn = (event_header >> 20) & 0xfff; // trigger counter bin
	tc[realGroup[group]] = tcn;

	// Check if all channels were active (if 8 channels active return 3072)
	int nsample = (event_header & 0xfff) / 3;
        
        //std::cout << "9" << std::endl;
        
	// Define time coordinate
	time[realGroup[group]][0] = 0.0;
	for( int i = 1; i < 1024; i++ ){
	  time[realGroup[group]][i] = float(i);
	  time[realGroup[group]][i] = float(tcal[realGroup[group]][(i-1+tcn)%1024] 
					    + time[realGroup[group]][i-1]);
	}      
        
        //std::cout << "10" << std::endl;
        
	//************************************
	// Read sample info for group
	//************************************      

	for ( int i = 0; i < nsample; i++ ) {
	  dummy = fread( &temp, sizeof(uint), 3, fpin );  
	  samples[0][i] =  temp[0] & 0xfff;
	  samples[1][i] = (temp[0] >> 12) & 0xfff;
	  samples[2][i] = (temp[0] >> 24) | ((temp[1] & 0xf) << 8);
	  samples[3][i] = (temp[1] >>  4) & 0xfff;
	  samples[4][i] = (temp[1] >> 16) & 0xfff;
	  samples[5][i] = (temp[1] >> 28) | ((temp[2] & 0xff) << 4);
	  samples[6][i] = (temp[2] >>  8) & 0xfff;
	  samples[7][i] =  temp[2] >> 20;	
	}
        
        //std::cout << "11" << std::endl;
        
	// Trigger channel
	for(int j = 0; j < nsample/8; j++){
	  fread( &temp, sizeof(uint), 3, fpin);  
	  samples[8][j*8+0] =  temp[0] & 0xfff;
	  samples[8][j*8+1] = (temp[0] >> 12) & 0xfff;
	  samples[8][j*8+2] = (temp[0] >> 24) | ((temp[1] & 0xf) << 8);
	  samples[8][j*8+3] = (temp[1] >>  4) & 0xfff;
	  samples[8][j*8+4] = (temp[1] >> 16) & 0xfff;
	  samples[8][j*8+5] = (temp[1] >> 28) | ((temp[2] & 0xff) << 4);
	  samples[8][j*8+6] = (temp[2] >>  8) & 0xfff;
	  samples[8][j*8+7] =  temp[2] >> 20;
	}
      }
      
      //std::cout << "12" << std::endl;
      //************************************
      // Loop over channels 0-8
      //************************************      

      for(int i = 0; i < 9; i++) {

	totalIndex = realGroup[group]*9 + i;
          
	// Do not analyze disabled channels
	if ( !config.hasChannel(totalIndex) ) {
	  for ( int j = 0; j < 1024; j++ ) {
	    raw[totalIndex][j] = 0; 
	    channel[totalIndex][j] = 0;
	  }
	  xmin[totalIndex] = 0.;
	  xminRestricted[totalIndex] = 0.;
	  amp [totalIndex] = 0.;
	  ampRestricted [totalIndex] = 0.;
	  base[totalIndex] = 0.;
	  integral[totalIndex] = 0.;
	  integralFull[totalIndex] = 0.;
	  gauspeak[totalIndex] = 0.;
	  sigmoidTime[totalIndex] = 0.;
	  linearTime0[totalIndex] = 0.;
	  linearTime15[totalIndex] = 0.;
	  linearTime30[totalIndex] = 0.;
	  linearTime45[totalIndex] = 0.;
	  linearTime60[totalIndex] = 0.;
	  risetime[totalIndex] = 0.;
	  constantThresholdTime[totalIndex] = 0.;
	  continue;
        }
          
	// Fill pulses
	if (DATTYPE) {
	  //std::cout << "13" << std::endl;
	  for ( int j = 0; j < 1024; j++ ) {
	    raw[totalIndex][j] = (short)(samples[i][j]);
	    channel[totalIndex][j] = (short)((double)(samples[i][j]) 
					     - (double)(off_mean[realGroup[group]][i][(j+tcn)%1024]));
	  }
	}
	///////////////////////////////////////
	//////// Starting here, dat and root file types both use all the following code 
	////////////////////////////////////////
      
	//std::cout << "14" << std::endl;
      
	// Make pulse shape graph
	TString pulseName = Form("pulse_event%d_group%d_ch%d", iEvent, realGroup[group], i);
	TGraphErrors* pulse = new TGraphErrors( GetTGraph( channel[totalIndex], time[realGroup[group]] ) );
      
	//std::cout << "15" << std::endl;
      
	// Only for dattype; base variable already extracted from root file if processing root type
      
	if (DATTYPE) {
      
	  // Estimate baseline
	  float baseline;
	  baseline = GetBaseline( pulse, 5 ,150, pulseName );
	  base[totalIndex] = baseline;
        
        
	  // Correct pulse shape for baseline offset
	  for(int j = 0; j < 1024; j++) {

	    float multiplier = config.getChannelMultiplicationFactor(totalIndex);
	    channel[totalIndex][j] = multiplier * (short)((double)(channel[totalIndex][j]) + baseline);
	  }
	}
      
	//std::cout << "16" << std::endl;
      
	//Apply HighPass Filter (clipping circuit)
	//HighPassFilter( channel[totalIndex], channelFilter[totalIndex],  time[realGroup[group]], 1000., 0.01 );

      
	// DRS-glitch finder: zero out bins which have large difference
	// with respect to neighbors in only one or two bins
	for(int j = 0; j < 1024; j++) {
	  short a0 = abs(channel[totalIndex][j-1]);
	  short a1 = abs(channel[totalIndex][j]);
	  short a2 = abs(channel[totalIndex][j+1]);
	  short a3 = abs(channel[totalIndex][j+2]);
        
	  if (j == 56 && a1 == 0 && a2 == 0) {
	    //std::cout << "PRE: " << i << " " << a[i] << " " << xmin << " " << a[i+1] << std::endl;}
	    //std::cout << "POST a value: " << *channel[totalIndex] << std::endl; 
	    if (*channel[totalIndex] == 5.) {
	      channel[totalIndex][j] = 0; }}
        
	  if ( ( a1>3*a0 && a2>3*a0 && a2>3*a3 && a1>30) )
	    {
	      channel[totalIndex][j] = 0;
	      channel[totalIndex][j+1] = 0;
	    }

	  if ( ( a1>3*a0 && a1>3*a2 && a1>30) )
	    channel[totalIndex][j] = 0;
	}
      
      
      
      
	// Find the absolute minimum. This is only used as a rough determination 
	// to decide if we'll use the early time samples
	// or the late time samples to do the baseline fit
	//std::cout << "---event "  << event << "-------ch#: " << totalIndex << std::endl;
	int index_min = FindMinAbsolute(1024, channel[totalIndex]); 
	int index_min_restricted = index_min;
	if (totalIndex > 0) {
	  index_min_restricted = FindMinAbsolute(1024, channel[totalIndex], xmin[0] , xmin[0] + 40 );	
	}
	      
	// Recreate the pulse TGraph using baseline-subtracted channel data
	delete pulse;
	pulse = new TGraphErrors( GetTGraph( channel[totalIndex], time[realGroup[group]] ) );
	xmin[totalIndex] = index_min;
	xminRestricted[totalIndex] = index_min_restricted;
	
	//std::cout << "INDEX MIN: " << index_min << std::endl;

	//if (doFilter && totalIndex == 4) {
	//	pulse = WeierstrassTransform( channel[totalIndex], time[realGroup[group]], pulseName, 2.0, false);
	//}
	float filterWidth = config.getFilterWidth(totalIndex);
	if (filterWidth) {
	  pulse = WeierstrassTransform( channel[totalIndex], time[realGroup[group]], 
					pulseName, filterWidth, false );
	}
      
      
	//std::cout << "18" << std::endl;
      
	//Compute Amplitude : use units V
	Double_t tmpAmp = 0.0;
	Double_t tmpTime = 0.0;
	pulse->GetPoint(index_min, tmpTime, tmpAmp);
	amp[totalIndex] = tmpAmp * (1.0 / 4096.0); 
	pulse->GetPoint(index_min_restricted, tmpTime, tmpAmp);
	ampRestricted[totalIndex] = tmpAmp * (1.0 / 4096.0); 

	// Get pulse integral
	if ( xmin[totalIndex] != 0 ) {
	  integral[totalIndex] = GetPulseIntegral( index_min , channel[totalIndex], "", 20, 40 );
	  integralFull[totalIndex] = GetPulseIntegral( index_min , channel[totalIndex], "full");
	}
	else {
	  integral[totalIndex] = 0.0;
	  integralFull[totalIndex] = 0.0;
	}
      
	//std::cout << "19" << std::endl;
      
	// Gaussian time stamp and constant-fraction fit
	Double_t min = 0.; Double_t low_edge = 0.; Double_t high_edge = 0.; Double_t y = 0.; 
	pulse->GetPoint(index_min, min, y);	
	pulse->GetPoint(index_min-4, low_edge, y); // time of the low edge of the fit range
	pulse->GetPoint(index_min+4, high_edge, y);  // time of the upper edge of the fit range	

	float timepeak   = 0;
	bool isTrigChannel = ( totalIndex == 8 || totalIndex == 17 
			       || totalIndex == 26 || totalIndex == 35 );
	float fs[6]; // constant-fraction fit output
	float fs_falling[6]; // falling exp timestapms
	float cft_low_range  = 0.30;
	float cft_high_range = 0.70;
	if ( !isTrigChannel ) {
	  if( drawDebugPulses ) {
	    if ( xmin[totalIndex] != 0.0 ) {
	      timepeak =  GausFit_MeanTime(pulse, low_edge, high_edge, pulseName); 
	      RisingEdgeFitTime( pulse, index_min, cft_low_range, cft_high_range, fs, event, "linearFit_" + pulseName, true );
	      //TailFitTime( pulse, index_min, fs_falling, event, "expoFit_" + pulseName, true );
	      //sigmoidTime[totalIndex] = SigmoidTimeFit( pulse, index_min, event, "linearFit_" + pulseName, true );
	      //fullFitTime[totalIndex] = FullFitScint( pulse, index_min, event, "fullFit_" + pulseName, true );
	    }
	  }
	  else {
	    if ( xmin[totalIndex] != 0.0 ) {
	      timepeak =  GausFit_MeanTime(pulse, low_edge, high_edge); 
	      RisingEdgeFitTime( pulse, index_min, cft_low_range, cft_high_range, fs, event, "linearFit_" + pulseName, false );
	      //TailFitTime( pulse, index_min, fs_falling, event, "expoFit_" + pulseName, false );
	      //sigmoidTime[totalIndex] = SigmoidTimeFit( pulse, index_min, event, "linearFit_" + pulseName, false );
	      //fullFitTime[totalIndex] = FullFitScint( pulse, index_min, event, "fullFit_" + pulseName, false );
	    }
	  }
	}

	else {
	  for ( int kk = 0; kk < 5; kk++ ) {
	    fs[kk] = -999;
	    fs_falling[kk] = -999;
	  }
	}

	_isRinging[totalIndex] = isRinging( index_min, channel[totalIndex] );
    	// if( isRinging( index_min, channel[totalIndex] ) )
	//   {
	//     std::cout << "event = " << event  << " channel = " << totalIndex << std::endl;
	//   };
	//std::cout << "20" << std::endl;
			
	// for output tree
	gauspeak[totalIndex] = timepeak;
	risetime[totalIndex] = fs[0];
	linearTime0[totalIndex] = fs[1];
	linearTime15[totalIndex] = fs[2];
	linearTime30[totalIndex] = fs[3];
	linearTime45[totalIndex] = fs[4];
	linearTime60[totalIndex] = fs[5];
	fallingTime[totalIndex] = fs_falling[0];
	constantThresholdTime[totalIndex] = ConstantThresholdTime( pulse, 200);

	//use algorithm ported from Nicola Minafra
	vector<double> tmpChannelVector;
	vector<double> tmpTimeVector;
	AlgorithmParameters par( 0.5, -0.1, -0.01, 700e6, 1e-3,-0.3,0.3,-0.4,0.4);
	for(int j = 0; j < 1024; j++) {
	  double tmpAmpSample = 0.0;
	  double tmpTimeSample = 0.0;
	  pulse->GetPoint(j, tmpAmpSample,tmpTimeSample);
	  tmpChannelVector.push_back(tmpAmpSample);
	  tmpTimeVector.push_back(tmpTimeSample);
	}
	exactTimeCFD[totalIndex] = ComputeExactTimeCFD(tmpChannelVector, tmpTimeVector, par);

	delete pulse;
      }
    
      if (DATTYPE) {
	dummy = fread( &event_header, sizeof(uint), 1, fpin); }
    }

    tree->Fill();
    nGoodEvents++;
  }

  //std::cout << "21" << std::endl;

  if (DATTYPE) {
    fclose(fpin);
  } else {
    rootInput->Close();
  }
  
  cout << "\nProcessed total of " << nGoodEvents << " events\n";

  file->Write();
  file->Close();

  return 0;
}



int graphic_init(){

  style = new TStyle("style", "style");

  style->SetLabelFont(132,"X");
  style->SetLabelFont(132,"Y");
  style->SetTitleFont(132,"X");
  style->SetTitleFont(132,"Y");
  style->SetTitleFont(132,"");
  style->SetTitleFontSize( 0.07);
  style->SetStatFont(132);
  style->GetAttDate()->SetTextFont(132);
  style->SetStatW(0.20);
  style->SetStatH(0.23);
  style->SetFuncColor(2);
  style->SetFuncWidth(2);
  style->SetLineWidth(2);
  style->SetOptFile(0);
  style->SetOptTitle(1);
  style->SetFrameBorderMode(0);
  style->SetCanvasBorderMode(0);
  style->SetPadBorderMode(0);
  style->SetTitleStyle(4000);
  style->SetPadColor(0);
  style->SetCanvasColor(0);
  style->SetTitleFillColor(0);
  style->SetTitleBorderSize(0);
  style->SetStatColor(0);
  style->SetStatBorderSize(1);
  style->SetOptStat("emri");
  style->SetOptFit(1);
  style->SetTitleOffset( 1.0,"Y");
  style->SetMarkerStyle(20);
  style->SetMarkerSize( 0.3);
  style->SetMarkerColor(4);

  style->cd();

  return 0;
}

