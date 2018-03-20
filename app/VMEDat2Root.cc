// C++ includes
#include <string>

// ROOT includes
#include <TROOT.h>

//LOCAL INCLUDES
#include "VMEAnalyzer.hh"

using namespace std;

int main(int argc, char **argv) {
  gROOT->SetBatch();

  std::string inputFileName = "something.dat";
  std::string outputFileName = "something.root";
  std::string configName = "config/15may2017.config";

  VMEAnalyzer analyzer(configName);
  analyzer.parse(inputFileName);
  analyzer.analyze(outputFileName);
  
  return 0;
}
