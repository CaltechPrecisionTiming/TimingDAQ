// C++ includes
#include <string>

// ROOT includes
#include <TROOT.h>

//LOCAL INCLUDES
#include "DRSclAnalyzer.hh"

using namespace std;

int main(int argc, char **argv) {
  gROOT->SetBatch();

  DRSclAnalyzer* analyzer = new DRSclAnalyzer();
  analyzer->GetCommandLineArgs(argc, argv);
  analyzer->RunEventsLoop();

  return 0;
}
