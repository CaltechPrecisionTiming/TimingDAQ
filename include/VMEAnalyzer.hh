#ifndef VMEAnalyzer_HH
#define VMEAnalyzer_HH

#include "DatAnalyzer.hh"

// This is the class that should be used for parsing and analyzing
// VME data files in .dat format.

class VMEAnalyzer : public DatAnalyzer {
    public:
        VMEAnalyzer(std::string configName) : DatAnalyzer(configName) {}
        void parse(std::string inName);
};

#endif
