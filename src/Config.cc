#include "Config.hh"

Config::Config(std::string fname) : filename(fname), _isValid(true),
                    _PerformGaussianFit(false), _PerformRisingEdgeFit(false) {
    std::string configLine;
    std::ifstream configStream(fname);
    if ( configStream.is_open() ) {
        while ( std::getline(configStream, configLine) ) {
            parseConfigLine(configLine);
        }
        configStream.close();
    }
    else {
        std::cerr << "[ERROR]: Could not open configuration file " << filename << std::endl;
        _isValid = false;
        exit(0);
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
            std::cout << "    Straight polarity set" << std::endl;
        }
        else if ( item == "-" ) {
            polarity.push_back(-1);
            std::cout << "    Inverse polarity set" << std::endl;
        }
        else {
          std::cerr << "[ERROR]: Invalid polarity for channel " << chNum << std::endl;
          exit(0);
        }

        // amplification
        nextConfigElement(ss, item);
        float amp = std::stof(item);
        amplification.push_back(amp);
        if ( amp ) {
            std::cout << "    Amplification of " << amp << " dB" << std::endl;
        }

        // attenuation
        nextConfigElement(ss, item);
        float att = std::stof(item);
        attenuation.push_back(att);
        if ( att ) {
            std::cout << "    Attenuation of " << att << " dB" << std::endl;
        }

        // algorithm
        nextConfigElement(ss, item);
        // DEBUG
        algorithm.push_back(item);
        if ( doGaussFit(chNum) ) {
            _PerformGaussianFit = true;
            std::cout << "    Perform gaussian pulse fit" << std::endl;
        }
        if ( doRisingEdgeFit(chNum) ) {
            _PerformRisingEdgeFit = true;
            std::cout << "    Perform constant-fraction fit" << std::endl;
        }

        // filter width
        nextConfigElement(ss, item);
        float width = std::stof(item);
        filterWidth.push_back(width);
        if ( width ) {
            std::cout << "    Weierstrass transform with filter width " << width << std::endl;
            std::cerr << "[ERROR]: Weierstrass transform not implemented yet" << std::endl;
            exit(0);
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
    return polarity[ind] * dBToAmplitudeRatio( amplification[ind] ) * dBToAmplitudeRatio( -attenuation[ind] );
}

bool Config::doGaussFit(unsigned int ch) {
    std::size_t found = algorithm[getChannelIndex(ch)].find("G");
    return found!=std::string::npos;
}

bool Config::doRisingEdgeFit(unsigned int ch) {
  std::size_t found = algorithm[getChannelIndex(ch)].find("Re");
  return found!=std::string::npos;
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
