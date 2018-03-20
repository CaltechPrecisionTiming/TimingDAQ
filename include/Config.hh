#ifndef Config_HH
#define Config_HH

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>

// This class reads a configuration file and extracts polarity, attenuation,
// and amplification factors for each digitizer channel.  
// 
// Config file lines should be of the following form (lines beginning with a '#' are ignored):
// CH  POLARITY  AMPLIFICATION  ATTENUATION (dB)  ALGORITHM  FILTER_WIDTH
// with:
// CH - channel number (integer)
// POLARITY - sign of the pulse ('+' or '-')
// AMPLIFICATION - in dB, amount of amplification that was applied to the input (float)
// ATTENUATION - in dB, amount of attenuation that was applied to the input (float)
// ALGORITHM - indicates the algorithm to run to extract pulse times (int):
//      0: no algorithm
//      1: gaussian fit
//      2: linear constant-fraction fit
//      3: gaussian and linear fits
// FILTER_WIDTH - gaussian kernel width for Weierstrass transform (gaussian filter).
//      If 0, no Weierstrass transform will be applied.

class Config {
    public:
        Config(std::string fname);

        // accessor functions
        unsigned int getChannelIndex(unsigned int ch);
        int getPolarity(unsigned int ch) { return polarity[getChannelIndex(ch)]; }
        float getAmplification(unsigned int ch) { return amplification[getChannelIndex(ch)]; }
        float getAttenuation(unsigned int ch) { return attenuation[getChannelIndex(ch)]; }
        int getAlgorithm(unsigned int ch) { return algorithm[getChannelIndex(ch)]; }
        float getFilterWidth(unsigned int ch) { return filterWidth[getChannelIndex(ch)]; }
        bool isValid() { return _isValid; }

        // time extraction algorithm
        bool doGaussFit(unsigned int ch);
        bool doRisingEdgeFit(unsigned int ch);

        // get overall multiplier including polarity, amplification, and attenuation
        float getChannelMultiplicationFactor(unsigned int ch);

        // returns true if at least one channel is present in the config
        bool hasChannels() { return channel.size(); }
        // returns true if the specified channel is present in the config
        bool hasChannel(unsigned int ch);

    private:
        // process one line of the config file
        void parseConfigLine(std::string line);
        // get next non-space token from config
        void nextConfigElement(std::stringstream &ss, std::string &item);
        // convert dB to amplitude ratio
        float dBToAmplitudeRatio(float dB);

        bool _isValid; // true if no illegal parameters found in config
        std::string filename; // config file path
        std::vector<unsigned int> channel; // channel number
        std::vector<int> polarity; // 1 or -1
        std::vector<float> amplification; // amplification factor in dB
        std::vector<float> attenuation; // attenuation factor in dB
        std::vector<int> algorithm; // bitmask indicating algorithm to run
        std::vector<float> filterWidth; // gaussian filter width
};

#endif
