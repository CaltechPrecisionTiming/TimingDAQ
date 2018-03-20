#ifndef VMEAnalyzer_HH
#define VMEAnalyzer_HH
#define VME_CHANNELS 36
#define VME_SAMPLES 1024

#include "DatAnalyzer.hh"

// This is the class that should be used for parsing and analyzing
// VME data files in .dat format.

class VMEAnalyzer : public DatAnalyzer {
    public:
        VMEAnalyzer(std::string configName) : DatAnalyzer(configName) {}
        void parse(std::string inName);
    private:
        short raw[VME_CHANNELS][VME_SAMPLES] = {0};
};

#endif
