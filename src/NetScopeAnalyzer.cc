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
    if ( (wave_attr.chMask >> i) & 0x1 ) {
      active_ch.push_back(i);
      cout << "Channel: " << i << " active" << endl;
    }
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
  //TODO: TO be precise should fill the time and channel arrays there
  //deleting the pre-existing ones
  for(int i = 0; i < NUM_SAMPLES; i++){
    time[0][i] = i* wave_attr.dt*1E9;
  }
}

// Fill tc, raw, time and amplitude
int NetScopeAnalyzer::GetChannelsMeasurement() {

    //Resetting the time value
    for(int i = 0; i < NUM_CHANNELS; i++){
      for(int j = 0; j < NUM_SAMPLES; j++){
        channel[i][j] = 0;
      }
    }

    // Loop over channels
    for(auto k : active_ch) {
      char event_header;
      fread(&event_header, sizeof(char), 1, bin_file);
      if (feof(bin_file)) {
        return -1;
      }
      else if (event_header != '#') {
        cout << Form("Event %d channel header: %X (%c)", i_evt, event_header, event_header) << endl;
        cout << "Not matching the expected character #" << endl;
        return -1;
      }

      char aux_N_bytes_wf_length[1];
      fread(aux_N_bytes_wf_length, sizeof(char), 1, bin_file);
      int N_bytes_wf_length = (int)(aux_N_bytes_wf_length[0] - '0');

      char* wf_length = new char[N_bytes_wf_length+1];
      fread(wf_length, sizeof(char), N_bytes_wf_length, bin_file);
      wf_length[N_bytes_wf_length] = '\0';
      int N_bytes_to_transfer = stoi(wf_length);
      delete [] wf_length;

      char* buffer = new char[N_bytes_to_transfer];
      fread(buffer, sizeof(char), N_bytes_to_transfer, bin_file);
      for(unsigned int i = 0; i < N_bytes_to_transfer; i++) {
        channel[k][i] = (buffer[i] - wave_attr.yoff[k]) * wave_attr.ymult[k] + wave_attr.yzero[k];
      }
      delete [] buffer;

    }
    char event_tail;
    fread(&event_tail, sizeof(char), 1, bin_file);

    if (event_tail == ':') {
      char aux_buf;
      while(!feof(bin_file) ) {
        fread(&aux_buf, sizeof(char), 1, bin_file);
        cout << aux_buf << flush;
        if(aux_buf == '#') {
          cout << endl << aux_buf << flush;
          fread(&aux_buf, sizeof(char), 1, bin_file);
          cout << aux_buf << flush;
          if(aux_buf-'0' == 4) {
            char buff[4+1];
            fread(buff, sizeof(char), 4, bin_file);
            buff[4] = '\0';
            cout << buff << endl;
            if(stoi(buff) == wave_attr.nPt) {
              fseek(bin_file, -(2+4)*sizeof(char), SEEK_CUR);
              return 0;
            }
          }

        }
      }

    }
    else if (event_tail != '\n') {
      cout << Form("Event %d tail: %X (%c)", i_evt, event_tail, event_tail) << endl;
      cout << "Not matching the expected event tail 0xA" << endl; //New line character '\n'=0xA
      return -1;
    }

    return 0;
}
