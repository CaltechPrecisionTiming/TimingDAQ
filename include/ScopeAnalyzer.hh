#ifndef ScopeAnalyzer_HH
#define ScopeAnalyzer_HH
#define SCOPE_SAMPLES 8
#define SCOPE_CHANNELS 4096
#include "DatAnalyzer.hh"

// This is the class that should be used for parsing and analyzing
// scope data files in .dat format.

class ScopeAnalyzer : public DatAnalyzer {
    public:
        ScopeAnalyzer(std::string configName) :
          DatAnalyzer(configName, SCOPE_CHANNELS, SCOPE_SAMPLES) {}
        void parse(std::string inName);
    private:
        short raw[SCOPE_CHANNELS][SCOPE_SAMPLES] = {0};
};

#endif
