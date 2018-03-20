#include "DatAnalyzer.hh"

DatAnalyzer::DatAnalyzer(std::string configName, int numChannels) : 
        NUM_CHANNELS(numChannels), config(configName),
        file(0), tree(0) {
    std::cout << "In DatAnalyzer constructor. Set NUM_CHANNELS to " << NUM_CHANNELS << std::endl;
    for (int j = 0; j < 1024; j++) {
        for (int i = 0; i < 4; i++) {
            time[i][j] = 0.;
        }
        for (int i = 0; i < NUM_CHANNELS; i++) {
            raw[i][j] = 0.;
        }
    }
    // TODO: init all variables
}

DatAnalyzer::~DatAnalyzer() {
    std::cout << "In DatAnalyzer destructor" << std::endl;
    if (file) {
        file->Close();
    }
}

void DatAnalyzer::initTree(std::string fname) {
    std::cout << "In DatAnalyzer::initTree" << std::endl;
    file = new TFile(fname.c_str(), "RECREATE");
    tree = new TTree("pulse", "Digitized waveforms");

    tree->Branch("time", time, "time[4][1024]/F");
    tree->Branch("raw", raw, Form("raw[%d][1024]/S", NUM_CHANNELS));
    // TODO: add all branches
}

void DatAnalyzer::analyze(std::string outName) {
    std::cout << "In DatAnalyzer::analyze" << std::endl;
    initTree(outName);
    // TODO: analysis code goes here
}
