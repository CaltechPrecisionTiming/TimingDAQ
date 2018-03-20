#ifndef DRSAnalyzer_HH
#define DRSAnalyzer_HH

#include "DatAnalyzer.hh"

// This is the class that should be used for parsing and analyzing
// DRS data files in .dat format.

class DRSAnalyzer : public DatAnalyzer {
    public:
        DRSAnalyzer(std::string configName);
        void parse(std::string inName);
    private:
        short raw[4][1024]; // redeclare it with only four channels
};

#endif
