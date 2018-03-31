#include "Configuration.hh"

Configuration::Configuration(std::string fname) {
    std::string configLine;
    std::ifstream configStream(fname);
    if ( configStream.is_open() ) {
        while ( getline(configStream, configLine) ) {
            parseConfigurationLine(configLine);
        }
        configStream.close();
    }
    else {
        std::cerr << "[ERROR]: Could not open configuration file " << fname << std::endl;
        exit(0);
    }
}

int Configuration::nextConfigurationElement(std::stringstream &ss, std::string &item) {
    while( std::getline(ss, item, ' ') ) {
        if ( item.size() && item != " " ) {
            return 1;
        }
    }
    return 0;
    // std::cout << "Warning in Configuration::nextConfigurationElement: next config element not found" << std::endl;
}

void Configuration::parseConfigurationLine(std::string line) {
    std::stringstream ss;
    ss.str(line);
    std::string item;

    if ( line[0] == '#' ) {
        return;
    }
    else if (line.substr(0, 8) == "Baseline") {
      nextConfigurationElement(ss, item);
      nextConfigurationElement(ss, item);
      baseline[0] = std::stoi(item);
      nextConfigurationElement(ss, item);
      baseline[1] = std::stoi(item);

      cout << "[CONFIG]: Baseline = [ " << baseline[0] << ", " << baseline[1] << " ]" << endl;
    }
    else if (line.substr(0, 16) == "ConstantFraction") {
      nextConfigurationElement(ss, item);
      constant_fraction.clear();

      cout << "[CONFIG]: ConstantFraction = { " << flush;
      while(nextConfigurationElement(ss, item)) {
        constant_fraction.push_back(0.01*std::stof(item));
        cout << 0.01*std::stof(item) << " " << flush;
      }
      cout << "}" << endl;
    }
    else if (line.substr(0, 5) == "z_DUT") {
      nextConfigurationElement(ss, item);
      z_DUT.clear();

      cout << "[CONFIG]: z_DUT = { " << flush;
      while(nextConfigurationElement(ss, item)) {
        z_DUT.push_back(std::stof(item));
        cout << std::stof(item) << " " << flush;
      }
      cout << "} [mm]" << endl;
    }
    else if (line[0] <= '9' && line[0] >= '0') {
      Channel aux_ch;

      nextConfigurationElement(ss, item);
      unsigned int chNum = std::stoi(item);
      aux_ch.N = chNum;
      std::cout << "[CONFIG]: Channel " << chNum << " activated" << std::endl;


      // polarity
      nextConfigurationElement(ss, item);
      if ( item == "+" ) {
          aux_ch.polarity = 1;
          std::cout << "    Negative pulse set (+: straight)" << std::endl;
      }
      else if ( item == "-" ) {
          aux_ch.polarity = -1;
          std::cout << "    Positive pulse set (-: inverse)" << std::endl;
      }
      else {
        std::cerr << "[ERROR]: Invalid polarity for channel " << chNum << std::endl;
        exit(0);
      }

      // amplification [dB]
      nextConfigurationElement(ss, item);
      float amp = std::stof(item);
      aux_ch.amplification = amp;
      if ( amp ) {
          std::cout << "    Amplification of " << amp << " dB" << std::endl;
      }

      // attenuation [dB]
      nextConfigurationElement(ss, item);
      float att = std::stof(item);
      aux_ch.attenuation = amp;
      if ( att ) {
          std::cout << "    Attenuation of " << att << " dB" << std::endl;
      }

      // algorithm
      nextConfigurationElement(ss, item);
      aux_ch.algorithm = item;
      cout << "    Algorithm: " << aux_ch.algorithm.Data() << endl;
      TString aux = aux_ch.algorithm(TRegexp("Re[0-9][0-9]-[0-9][0-9]"));
      if ( aux.Length() == 7 ) {
        aux_ch.re_bounds[0] = stof(aux(2,2).Data()) / 100.;
        aux_ch.re_bounds[1] = stof(aux(5,2).Data()) / 100.;
        if( aux_ch.re_bounds[0] > aux_ch.re_bounds[1] ) {
          cerr << "[ERROR]: Rising edge bounds in Config file wrong (maybe swapped?)" << endl;
          exit(0);
        }
      }
      aux = aux_ch.algorithm(TRegexp("G[0-9][0-9]"));
      if ( aux.Length() == 3 ) {
        aux_ch.gaus_fraction = stof(aux(1,2).Data()) / 100.;
      }
      for(unsigned int i = 1; i <= 3; i++) {
        if ( aux_ch.algorithm.Contains(Form("LP%d", i)) ) {
          aux_ch.PL_deg.push_back(i);
        }
      }

      // filter width
      nextConfigurationElement(ss, item);
      float width = std::stof(item);
      aux_ch.weierstrass_filter_width = width;
      if ( width ) {
          std::cout << "    Weierstrass transform with filter width " << width << std::endl;
          std::cerr << "[ERROR]: Weierstrass transform not implemented yet" << std::endl;
          exit(0);
      }

      channels[chNum] = aux_ch;
    }

}

float Configuration::getChannelMultiplicationFactor(unsigned int ch) {
  float out = channels[ch].polarity;
  out *= pow(10, channels[ch].amplification/20.);
  out *= pow(10, -channels[ch].attenuation/20.);
  return out;
}

bool Configuration::hasChannel(unsigned int ch) {
  for(auto it = channels.begin(); it != channels.end(); ++it) {
    if( it->first == ch) return true;
  }
  return false;
}

bool Configuration::isValid() {
  if (channels.size() > 0) return true;
  else return false;
}
