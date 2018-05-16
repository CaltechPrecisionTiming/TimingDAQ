#include "Configuration.hh"

Configuration::Configuration(std::string fname, bool verb) {
    verbose = verb;

    std::string configLine;
    std::ifstream configStream(fname);
    if ( configStream.is_open() ) {
        while ( getline(configStream, configLine) ) {
            parseConfigurationLine(configLine);
        }
        configStream.close();
    }
    else {
        std::cerr << "[ERROR] Could not open configuration file " << fname << std::endl;
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
    // cout << "Warning in Configuration::nextConfigurationElement: next config element not found" << std::endl;
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

      if( verbose ) { cout << "[CONFIG] Baseline = [ " << baseline[0] << ", " << baseline[1] << " ]" << endl;}
    }
    else if (line.substr(0, 16) == "ConstantFraction") {
      nextConfigurationElement(ss, item);
      constant_fraction.clear();

      if( verbose ) { cout << "[CONFIG] ConstantFraction = { " << flush;}
      while(nextConfigurationElement(ss, item)) {
        constant_fraction.push_back(0.01*std::stof(item));
        if( verbose ) { cout << 0.01*std::stof(item) << " " << flush;}
      }
      if( verbose ) { cout << "}" << endl;}
    }
    else if (line.substr(0, 17) == "ConstantThreshold") {
      nextConfigurationElement(ss, item);
      constant_threshold.clear();

      if( verbose ) { cout << "[CONFIG] ConstantThreshold = { " << flush;}
      while(nextConfigurationElement(ss, item)) {
        constant_threshold.push_back(std::stof(item));
        if( verbose ) { cout << std::stof(item) << " " << flush;}
      }
      if( verbose ) { cout << "} [mV]" << endl;}
    }
    else if (line.substr(0, 5) == "z_DUT") {
      nextConfigurationElement(ss, item);
      z_DUT.clear();

      if( verbose ) { cout << "[CONFIG] z_DUT = { " << flush;}
      while(nextConfigurationElement(ss, item)) {
        z_DUT.push_back(std::stof(item));
        if( verbose ) { cout << std::stof(item) << " " << flush;}
      }
      if( verbose ) { cout << "} [mm]" << endl;}
    }
    else if (line[0] <= '9' && line[0] >= '0') {
      Channel aux_ch;

      nextConfigurationElement(ss, item);
      unsigned int chNum = std::stoi(item);
      aux_ch.N = chNum;
      if( verbose ) { cout << "[CONFIG] Channel " << chNum << " activated" << std::endl;}


      // polarity
      nextConfigurationElement(ss, item);
      if ( item == "+" ) {
          aux_ch.polarity = 1;
          if( verbose ) { cout << "    Negative pulse set (+: straight)" << std::endl;}
      }
      else if ( item == "-" ) {
          aux_ch.polarity = -1;
          if( verbose ) { cout << "    Positive pulse set (-: inverse)" << std::endl;}
      }
      else {
        std::cerr << "[ERROR] Invalid polarity for channel " << chNum << std::endl;
        exit(0);
      }

      // amplification [dB]
      nextConfigurationElement(ss, item);
      float amp = std::stof(item);
      aux_ch.amplification = amp;
      if ( amp ) {
          if( verbose ) { cout << "    Amplification of " << amp << " dB" << std::endl;}
      }

      // attenuation [dB]
      nextConfigurationElement(ss, item);
      float att = std::stof(item);
      aux_ch.attenuation = amp;
      if ( att ) {
          if( verbose ) { cout << "    Attenuation of " << att << " dB" << std::endl;}
      }

      // algorithm
      nextConfigurationElement(ss, item);
      aux_ch.algorithm = item;
      if( verbose ) { cout << "    Algorithm: " << aux_ch.algorithm.Data() << endl;}
      TString aux = aux_ch.algorithm(TRegexp("Re[0-9][0-9]-[0-9][0-9]"));
      if ( aux.Length() == 7 ) {
        aux_ch.re_bounds[0] = stof(aux(2,2).Data()) / 100.;
        aux_ch.re_bounds[1] = stof(aux(5,2).Data()) / 100.;
        if( aux_ch.re_bounds[0] > aux_ch.re_bounds[1] ) {
          cerr << "[ERROR] Rising edge bounds in Config file wrong (maybe swapped?)" << endl;
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
          if( verbose ) { cout << "    Weierstrass transform with filter width " << width << std::endl;}
          std::cerr << "[ERROR] Weierstrass transform not implemented yet" << std::endl;
          exit(0);
      }

      channels[chNum] = aux_ch;
    }
    else if (line.substr(0, 3) == "Npe")
    {
      nextConfigurationElement(ss, item);
      nextConfigurationElement(ss, item);
      Npe = std::stoi(item);
      if( verbose ){ std::cout << "[VERBOSE] Npe = " << Npe << std::endl;}
    }
    else if (line.substr(0, 26) == "ScintillationDecayConstant")
    {
      nextConfigurationElement(ss, item);
      nextConfigurationElement(ss, item);
      scintillation_decay_constant = std::stof(item);
      if( verbose ){ std::cout << "[VERBOSE]  scintillation_decay_constant = " << scintillation_decay_constant << std::endl;}
    }
    else if (line.substr(0, 21) == "ScintillationRisetime")
    {
      nextConfigurationElement(ss, item);
      nextConfigurationElement(ss, item);
      scintillation_risetime = std::stof(item);
      if( verbose ){ std::cout << "[VERBOSE] scintillation_risetime = " << scintillation_risetime << std::endl;}
    }
    else if (line.substr(0, 28) == "SinglePhotonRisetimeResponse")
    {
      nextConfigurationElement(ss, item);
      nextConfigurationElement(ss, item);
      single_photon_risetime_response = std::stof(item);
      if( verbose ){ std::cout << "[VERBOSE] single_photon_risetime_response = " << single_photon_risetime_response << std::endl;}
    }
    else if (line.substr(0, 29) == "SinglePhotonDecaytimeResponse")
    {
      nextConfigurationElement(ss, item);
      nextConfigurationElement(ss, item);
      single_photon_decaytime_response = std::stof(item);
      if( verbose ){ std::cout << "[VERBOSE] single_photon_decaytime_response = " << single_photon_decaytime_response << std::endl;}
    }
    else if (line.substr(0, 16) == "HighPassFilterRC")
    {
      nextConfigurationElement(ss, item);
      nextConfigurationElement(ss, item);
      high_pass_filter_RC = std::stof(item);
      if( verbose ){ std::cout << "[VERBOSE] high_pass_filter_RC = " << high_pass_filter_RC << std::endl;}
    }
    else if (line.substr(0, 13) == "DarkCountRate")
    {
      nextConfigurationElement(ss, item);
      nextConfigurationElement(ss, item);
      DCR = std::stof(item);
      if( verbose ){ std::cout << "[VERBOSE] DCR = " << DCR << std::endl;}
    }
    else if (line.substr(0, 10) == "nThreshold")
    {
      nextConfigurationElement(ss, item);
      nextConfigurationElement(ss, item);
      n_threshold = std::stoi(item);
      if( verbose ){ std::cout << "[VERBOSE] n_threshold = " << n_threshold << std::endl;}
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
