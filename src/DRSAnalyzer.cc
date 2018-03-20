#include "DRSAnalyzer.hh"

DRSAnalyzer::DRSAnalyzer(std::string configName) :
    DatAnalyzer(configName, 4) { } // specify only 4 channels

void DRSAnalyzer::parse(std::string inName) {
    std::cout << "In DRSAnalyzer::parse" << std::endl;
    // TODO: implement DRS parsing
}
