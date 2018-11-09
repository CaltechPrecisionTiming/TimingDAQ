// C++ includes
#include <string>
#include <assert.h>

// ROOT includes
#include <TROOT.h>

//LOCAL INCLUDES
#include "NetScopeStandaloneAnalyzer.hh"

using namespace std;

int main(int argc, char **argv) {
  gROOT->SetBatch();

  NetScopeStandaloneAnalyzer* analyzer = new NetScopeStandaloneAnalyzer();
  analyzer->GetCommandLineArgs(argc, argv);
  analyzer->RunEventsLoop();

  //patch for some lost memory situation
  assert(false);

  return 0;
}
