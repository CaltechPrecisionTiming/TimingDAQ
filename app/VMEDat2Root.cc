// C++ includes
#include <string>
#include <assert.h>

// ROOT includes
#include <TROOT.h>

//LOCAL INCLUDES
#include "VMEAnalyzer.hh"

using namespace std;

int main(int argc, char **argv) {
  gROOT->SetBatch();

  VMEAnalyzer* analyzer = new VMEAnalyzer();
  analyzer->GetCommandLineArgs(argc, argv);
  analyzer->LoadCalibration();
  analyzer->RunEventsLoop();

  //patch for some lost memory situation
  #ifndef __APPLE__
    assert(false);
  #endif

  return 0;
}
