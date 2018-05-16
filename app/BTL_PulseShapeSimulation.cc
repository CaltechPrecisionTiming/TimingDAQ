// C++ includes
#include <string>

// ROOT includes
#include <TROOT.h>

//LOCAL INCLUDES
#include "SiPM_SimAnalyzer.hh"
#include "PulseShape.hh"

bool PulseShape::_info    = false;
bool PulseShape::_debug   = false;
bool PulseShape::_warning = false;

using namespace std;

int main(int argc, char **argv) {
  gROOT->SetBatch();

  SiPM_SimAnalyzer* analyzer = new SiPM_SimAnalyzer();
  analyzer->GetCommandLineArgs(argc, argv);
  analyzer->RunEventsLoop();

  return 0;
}
