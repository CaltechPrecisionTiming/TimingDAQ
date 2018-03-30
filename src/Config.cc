#include "Config.hh"

#include <iostream>
#include <stdexcept>      // std::invalid_argument
#include <bitset>         // std::bitset
#include <string>         // std::string

using namespace std;

Config::Config(std::string fname) : filename(fname), _isValid(true) {
    std::string configLine;
    std::ifstream configStream(fname);
    if ( configStream.is_open() ) {
        while ( getline(configStream, configLine) ) {
            parseConfigLine(configLine);
        }
        configStream.close();
    }
    else {
        std::cerr << "Could not open configuration file " << filename << std::endl; 
        _isValid = false;
    }
}

void Config::nextConfigElement(std::stringstream &ss, std::string &item) {
    while( std::getline(ss, item, ' ') ) {
        if ( item.size() && item != " " ) {
            return;
        }
    }
    std::cout << "Warning in Config::nextConfigElement: next config element not found" << std::endl;
}

void Config::parseConfigLine(std::string line) {
    std::stringstream ss;
    ss.str(line);
    std::string item;
    
    try {
        // channel number (and check for commented line)
        int chNum = -1;
        nextConfigElement(ss, item);
        if ( item.at(0) == '#' ) {
            return;
        }
        else {
            chNum = std::stoi(item);
            channel.push_back(chNum);
            std::cout << "Config: channel " << chNum << " activated" << std::endl;
        }

        // polarity
        nextConfigElement(ss, item);
        if ( item == "+" ) {
            polarity.push_back(1);
        }
        else {
            polarity.push_back(-1);
            std::cout << "Config: inverse polarity for channel " << chNum << std::endl;
        }

        // amplification
        nextConfigElement(ss, item);
        float amp = std::stof(item);
        amplification.push_back(amp);
        if ( amp ) {
            std::cout << "Config: amplification of " << amp << " dB for channel " 
                << chNum << std::endl;
        }

        // attenuation
        nextConfigElement(ss, item);
        float att = std::stof(item);
        attenuation.push_back(att);
        if ( att ) {
            std::cout << "Config: attenuation of " << att << " dB for channel " 
                << chNum << std::endl;
        }

        // algorithm
        nextConfigElement(ss, item);
        int alg = std::stoi(item);
        algorithm.push_back(alg);
        if ( doGaussFit(chNum) ) {
            std::cout << "Config: will perform gaussian pulse fit in channel " 
                << chNum << std::endl;
        }
        if ( doRisingEdgeFit(chNum) ) {
            std::cout << "Config: will perform constant-fraction fit in channel " 
                << chNum << std::endl;
        }

        // filter width
        nextConfigElement(ss, item);
        float width = std::stof(item);
        filterWidth.push_back(width);
        if ( width ) {
            std::cout << "Config: will apply Weierstrass transform with filter width " 
                << width << " to channel " << chNum << std::endl;
        }
    }
    catch (std::invalid_argument) {
        std::cerr << "Illegal parameter found in config!  Configuration is not valid." << std::endl;
        _isValid = false;
    }
}

float Config::dBToAmplitudeRatio(float dB) {
    return pow(10, dB/20.);
}

float Config::getChannelMultiplicationFactor(unsigned int ch) {
    unsigned int ind = getChannelIndex(ch);
    return polarity[ind] * dBToAmplitudeRatio( amplification[ind] ) 
        * dBToAmplitudeRatio( -attenuation[ind] );
}

bool Config::doGaussFit(unsigned int ch) {
    return algorithm[getChannelIndex(ch)] & 0x1;
}

bool Config::doRisingEdgeFit(unsigned int ch) {
    return algorithm[getChannelIndex(ch)] & 0x2;
}

unsigned int Config::getChannelIndex(unsigned int ch) {
    for ( unsigned int i = 0; i < channel.size(); i++ ) {
        if (channel[i] == ch) {
            return i;
        }
    }
    std::cerr << "Config error: channel " << ch << " requested but not found in config!" << std::endl;
    return -1;
}

bool Config::hasChannel(unsigned int ch) {
    for ( unsigned int i = 0; i < channel.size(); i++ ) {
        if (channel[i] == ch) {
            return true;
        }
    }
    return false;
}
