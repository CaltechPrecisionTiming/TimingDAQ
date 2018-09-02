// C++ includes
#include <string>

// ROOT includes
#include <TROOT.h>

//LOCAL INCLUDES
#include "ETL_ASIC_Analyzer.hh"

using namespace std;

int main(int argc, char **argv) {
  gROOT->SetBatch();

  ETL_ASIC_Analyzer* analyzer = new ETL_ASIC_Analyzer();
  analyzer->GetCommandLineArgs(argc, argv);
  analyzer->RunEventsLoop();

  return 0;
}
