#include "NetScopeAnalyzer.hh"
#define BUFSIZE 8192

using namespace std;

void NetScopeAnalyzer::GetCommandLineArgs(int argc, char **argv){
  DatAnalyzer::GetCommandLineArgs(argc, argv);
}


void NetScopeAnalyzer::InitLoop(){
  DatAnalyzer::InitLoop();

  // get the file header and setup the waveform attribute
  char headerbuf[BUFSIZE];
  fgets(headerbuf, BUFSIZE, bin_file);

  sscanf(headerbuf,
  "waveform_attribute:"
  "     chMask  = 0x%02x"
  "     nPt     = %zd"
  "     nFrames = %zd"
  "     dt      = %g"
  "     t0      = %g"
  "     ymult   = %g %g %g %g"
  "     yoff    = %g %g %g %g"
  "     yzero   = %g %g %g %g",
  &(wave_attr.chMask), &(wave_attr.nPt), &(wave_attr.nFrames), &(wave_attr.dt), &(wave_attr.t0),
  &(wave_attr.ymult[0]), &(wave_attr.ymult[1]), &(wave_attr.ymult[2]), &(wave_attr.ymult[3]),
  &(wave_attr.yoff[0]), &(wave_attr.yoff[1]), &(wave_attr.yoff[2]), &(wave_attr.yoff[3]),
  &(wave_attr.yzero[0]), &(wave_attr.yzero[1]), &(wave_attr.yzero[2]), &(wave_attr.yzero[3]));

  for( unsigned int i = 0; i < 4; i++ ) {
    if ( (wave_attr.chMask >> i) & 0x1 ) active_ch.push_back(i);
  }

  printf("Waveform Attribute:\n"
  "     chMask  = 0x%02x\n"
  "     nPt     = %zd\n"
  "     nFrames = %zd\n"
  "     dt      = %g\n"
  "     t0      = %g\n"
  "     ymult   = %g %g %g %g\n"
  "     yoff    = %g %g %g %g\n"
  "     yzero   = %g %g %g %g\n",
  (wave_attr.chMask), (wave_attr.nPt), (wave_attr.nFrames), (wave_attr.dt), (wave_attr.t0),
  (wave_attr.ymult[0]), (wave_attr.ymult[1]), (wave_attr.ymult[2]), (wave_attr.ymult[3]),
  (wave_attr.yoff[0]), (wave_attr.yoff[1]), (wave_attr.yoff[2]), (wave_attr.yoff[3]),
  (wave_attr.yzero[0]), (wave_attr.yzero[1]), (wave_attr.yzero[2]), (wave_attr.yzero[3]));

  // Setting the time value
  for(int i = 0; i < NUM_SAMPLES; i++){
    time[0][i] = i* wave_attr.dt;
  }

  exit(0);
}

// Fill tc, raw, time and amplitude
int NetScopeAnalyzer::GetChannelsMeasurement() {

    //Resetting the time value
    for(int i = 0; i < NUM_CHANNELS; i++){
      for(int j = 0; j < NUM_SAMPLES; j++){
        channel[i][j] = 0;
      }
    }
    float ADC = (1.0 / 4096.0);

    char sample[16*BUFSIZE];

    fread(buf, sizeof(char), BUFSIZE, bin_file);

    channel[iCh][i] = ((waveformBuf[j * wave_attr.nPt + i]
           - wave_attr.yoff[iCh])
      * wave_attr.ymult[iCh]
      + wave_attr.yzero[iCh]) / ADC;


    exit(0);
    return 0;
}

// void NetScopeAnalyzer::Analyze(){
//   if(pixel_input_file_path != ""){
//     xIntercept = -999;
//     yIntercept = -999;
//     xSlope = -999;
//     ySlope = -999;
//     for(unsigned int i = 0; i < config->z_DUT.size(); i++) {
//       x_DUT[i] = -999;
//       y_DUT[i] = -999;
//     }
//     chi2 = -999.;
//     ntracks = 0;
//
//     while (idx_px_tree < entries_px_tree && i_evt >= pixel_event->trigger) {
//       pixel_tree->GetEntry(idx_px_tree);
//       if (pixel_event->trigger == i_evt) {
//         if(ntracks == 0) {
//           xIntercept = 1e-3*pixel_event->xIntercept; //um to mm
//           yIntercept = 1e-3*pixel_event->yIntercept;
//           xSlope = pixel_event->xSlope;
//           ySlope = pixel_event->ySlope;
//           for(unsigned int i = 0; i < config->z_DUT.size(); i++) {
//             x_DUT[i] = xIntercept + xSlope*(config->z_DUT[i]);
//             y_DUT[i] = yIntercept + ySlope*(config->z_DUT[i]);
//           }
//           chi2 = pixel_event->chi2;
//         }
//       	ntracks++;
//         idx_px_tree++;
//       }
//       else if (i_evt > pixel_event->trigger) {
//         cout << "[ERROR] Pixel tree not ordered" << endl;
//         exit(0);
//       }
//     }
//
//   }
//
//   DatAnalyzer::Analyze();
// }
