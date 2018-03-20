#ifndef DRSAnalyzer_HH
#define DRSAnalyzer_HH
#define DRS_CHANNELS 4
#define DRS_SAMPLES 1024

#include "DatAnalyzer.hh"

// This is the class that should be used for parsing and analyzing
// DRS data files in .dat format.

class DRSAnalyzer : public DatAnalyzer {
    public:
        DRSAnalyzer(std::string configName) :
            DatAnalyzer(configName, DRS_CHANNELS) {
              // TODO: Complain if the config is giving a larget or smallr number of channel (Maybe in the base class)
            } // specify only DRS_CHANNELS channels
        void parse(std::string inName);
    private:
        short raw[DRS_CHANNELS][DRS_SAMPLES] = {0};
};

#endif
