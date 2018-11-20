// C++ includes
#include <string>
#include <assert.h>

// ROOT includes
#include <TROOT.h>

//LOCAL INCLUDES
#include "DT5742Analyzer.hh"

using namespace std;

int main(int argc, char **argv) {
  gROOT->SetBatch();

  DT5742Analyzer* analyzer = new DT5742Analyzer();
  analyzer->GetCommandLineArgs(argc, argv);
  analyzer->LoadCalibration();
  analyzer->RunEventsLoop();

  //patch for some lost memory situation
  assert(false);

  return 0;
}
