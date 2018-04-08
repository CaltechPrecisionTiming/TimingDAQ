// C++ includes
#include <string>
#include <assert.h>

// ROOT includes
#include <TROOT.h>

//LOCAL INCLUDES
#include "NetScopeAnalyzer.hh"

using namespace std;

int main(int argc, char **argv) {
  gROOT->SetBatch();

  NetScopeAnalyzer* analyzer = new NetScopeAnalyzer();
  analyzer->GetCommandLineArgs(argc, argv);
  analyzer->RunEventsLoop();

  //patch for some lost memory situation
  assert(false);

  return 0;
}
