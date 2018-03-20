#include "DatAnalyzer.hh"

DatAnalyzer::DatAnalyzer(std::string configName, int numChannels, int numSamples) :
        NUM_CHANNELS(numChannels), NUM_SAMPLES(numSamples), config(configName),
        file(0), tree(0) {
    std::cout << "In DatAnalyzer constructor. Set NUM_CHANNELS to " << NUM_CHANNELS << std::flush;
    std::cout << ". Set NUM_SAMPLES to " << NUM_SAMPLES << std::endl;
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

    tree->Branch("time", time, Form("time[4][%d]/F", NUM_SAMPLES));
    tree->Branch("raw", raw, Form("raw[%d][%d]/S", NUM_CHANNELS, NUM_SAMPLES));
    // TODO: add all branches
}

void DatAnalyzer::analyze(std::string outName) {
    std::cout << "In DatAnalyzer::analyze" << std::endl;
    initTree(outName);
    // TODO: analysis code goes here
}
