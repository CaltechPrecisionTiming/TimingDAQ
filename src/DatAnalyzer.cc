#include "DatAnalyzer.hh"

#define DEFAULT_FOR_EMPTY_CH 0
#define THR_OVER_NOISE 3

using namespace std;

DatAnalyzer::DatAnalyzer(int numChannels, int numTimes, int numSamples, int res, float scale, int numFsamples) :
        NUM_CHANNELS(numChannels), NUM_TIMES(numTimes), NUM_SAMPLES(numSamples), NUM_F_SAMPLES(numFsamples),
        DAC_RESOLUTION(res), DAC_SCALE(scale),
        file(0), tree(0) {

    AUX_time = new float[numTimes*numSamples];
    AUX_channel = new float[numChannels*numSamples];

    time    = new float*[numTimes];
    channel = new float*[numChannels];

    if ( numFsamples > 0 )
    {
      AUX_channel_spectrum = new float[numChannels*numFsamples];
      channel_spectrum     = new float*[numFsamples];
      frequency = new float[numFsamples];
    }

    for(unsigned int i=0; i<numChannels; i++)
    {
      channel[i] = &(AUX_channel[i*numSamples]);
      if(i<numTimes) time[i] = &(AUX_time[i*numSamples]);
      if ( numFsamples ) channel_spectrum[i] = &(AUX_channel_spectrum[i*numFsamples]);
    }

    if ( numFsamples > 0 )
    {
      float f_step = (F_HIGH-F_LOW)/float(numFsamples);
      for (unsigned int i = 0; i < numFsamples; i++)
      {
        frequency[i] = F_LOW + f_step*float(i);
      }
    }

    if ( verbose ) {
      cout << "NUM_CHANNELS: " << NUM_CHANNELS << endl;
      cout << "NUM_TIMES: " << NUM_TIMES << endl;
      cout << "NUM_SAMPLES: " << NUM_SAMPLES << endl;
      cout << "NUM_F_SAMPLES: " << NUM_F_SAMPLES << endl;
      cout << "DAC_SCALE : " << DAC_SCALE << "\n";
      cout << "DAC_RESOLUTION : " << DAC_RESOLUTION << "\n";
    }
}

DatAnalyzer::~DatAnalyzer()
{
  cout << "In DatAnalyzer destructor" << endl;
  if (file)
  {
    file->Close();
  }
};

/*
**********************************************
Main Method of the class. Analyzes all events
Stores all values of the time stamps and other
analysis quantities.
**********************************************
*/
void DatAnalyzer::Analyze(){
  /*************************************
  LOOP OVER CHANNELS
  **************************************/
  for(unsigned int i=0; i<NUM_CHANNELS; i++) {
    ResetVar(i);
    if ( !config->hasChannel(i) ) {
      continue;
    }
    TString name = Form("pulse_event%d_ch%d", i_evt, i);
    // Get the attenuation/amplification scale factor and convert ADC counts to mV
    //float scale_factor = (30.0 * DAC_SCALE / (float)DAC_RESOLUTION) * config->getChannelMultiplicationFactor(i);
    float scale_factor = (1000.0 * DAC_SCALE / (float)DAC_RESOLUTION) * config->getChannelMultiplicationFactor(i);

    //cout << "check : " << scale_factor << " = " << DAC_SCALE << " " << DAC_RESOLUTION << " " << config->getChannelMultiplicationFactor(i) << "\n";

    // ------- Get baseline ------
    float baseline = 0;
    unsigned int bl_st_idx = config->baseline[0];
    unsigned int bl_lenght = config->baseline[1];
    for(unsigned int j=bl_st_idx; j<(bl_st_idx+bl_lenght); j++) {
      baseline += channel[i][j];
    }
    baseline /= (float) bl_lenght;
    var["baseline"][i] = scale_factor * baseline;
    // ------------- Get minimum position, max amplitude and scale the signal
    unsigned int idx_min = 0;
    float amp = 0;
    for(unsigned int j=0; j<NUM_SAMPLES; j++) {
      channel[i][j] = scale_factor * (channel[i][j] - baseline);//baseline subtraction
      //channel[i][j] = scale_factor * channel[i][j];//no baseline subtraction
      if(( j>bl_st_idx+bl_lenght && j<(int)(0.9*NUM_SAMPLES) && fabs(channel[i][j]) > fabs(amp)) || j == bl_st_idx+bl_lenght) {
      //if(( j<bl_st_idx+bl_lenght && j>10 && fabs(channel[i][j]) > fabs(amp))) {
        idx_min = j;
        amp = channel[i][j];
      }
    }
    var["t_peak"][i] = time[GetTimeIndex(i)][idx_min];
    var["amp"][i] = -amp;

    float baseline_RMS = 0;
    var["noise"][i] = channel[i][bl_st_idx+5];
    for(unsigned int j=bl_st_idx; j<=(bl_st_idx+bl_lenght); j++) {
      baseline_RMS += channel[i][j]*channel[i][j];
    }
    baseline_RMS = sqrt(baseline_RMS/bl_lenght);
    var["baseline_RMS"][i] = baseline_RMS;


    // --------------- Define pulse graph
    float * yerr = new float[NUM_SAMPLES];
    for(unsigned j = 0; j < NUM_SAMPLES; j++) yerr[j] = 0 * var["baseline_RMS"][i];
    TGraphErrors* pulse = new TGraphErrors(NUM_SAMPLES, time[GetTimeIndex(i)], channel[i], 0, yerr);
    pulse->SetNameTitle("g_"+name, "g_"+name);

    // Variables used both by analysis and pulse drawer
    unsigned int j_90_pre = 0, j_10_pre = 0;
    unsigned int j_90_post = 0, j_10_post = 0;
    unsigned int j_area_pre = 0, j_area_post = 0;
    vector<float*> coeff_poly_fit;
    vector<pair<int, int>> poly_bounds;
    float Re_b, Re_slope;

    bool fittable = true;
    fittable *= idx_min < (int)(NUM_SAMPLES*0.8);
    fittable *= fabs(amp) > 8 * baseline_RMS;
    fittable *= fabs(channel[i][idx_min+1]) > 4*baseline_RMS;
    fittable *= fabs(channel[i][idx_min-1]) > 4*baseline_RMS;
    fittable *= fabs(channel[i][idx_min+2]) > 3*baseline_RMS;
    fittable *= fabs(channel[i][idx_min-2]) > 3*baseline_RMS;
    // fittable *= fabs(channel[i][idx_min+3]) > 2*baseline_RMS;
    // fittable *= fabs(channel[i][idx_min-3]) > 2*baseline_RMS;
    if( fittable  && !config->channels[i].algorithm.Contains("None")) {
      // Correct the polarity if wrong
      if( amp > 0 && config->channels[i].counter_auto_pol_switch >= 0 ) {
        config->channels[i].polarity *= -1;
        amp = -amp;
        var["amp"][i] = -amp;
        scale_factor = -scale_factor;
        var["baseline"][i] = scale_factor * baseline;
        for(unsigned int j=0; j<NUM_SAMPLES; j++) {
          channel[i][j] = -channel[i][j];
        }
        delete pulse;
        pulse = new TGraphErrors(NUM_SAMPLES, time[GetTimeIndex(i)], channel[i], 0, yerr);
        pulse->SetNameTitle("g_"+name, "g_"+name);

        if ( config->channels[i].counter_auto_pol_switch == 10 ) {
          cout << "[WARNING] Channel " << i << " automatic polarity switched more than 10 times" << endl;
        }
        config->channels[i].counter_auto_pol_switch ++;
      }

      /*
      ***********************************
      //Get 10% of the amplitude crossings
      ************************************
      */
      j_10_pre = GetIdxFirstCross(amp*0.1, channel[i], idx_min, -1);
      j_10_post = GetIdxFirstCross(amp*0.1, channel[i], idx_min, +1);

      // -------------- Integrate the pulse
      j_area_pre = GetIdxFirstCross(amp*0.05, channel[i], idx_min, -1);
      j_area_post = GetIdxFirstCross(0. , channel[i], idx_min, +1);
      var["integral"][i] = GetPulseIntegral(channel[i], time[GetTimeIndex(i)], j_area_pre, j_area_post);
      var["intfull"][i] = GetPulseIntegral(channel[i], time[GetTimeIndex(i)], 5, NUM_SAMPLES-5);

      // -------------- Compute rise and falling time
      j_90_pre = GetIdxFirstCross(amp*0.9, channel[i], j_10_pre, +1);
      var["risetime"][i] = time[GetTimeIndex(i)][j_90_pre] - time[GetTimeIndex(i)][j_10_pre];
      j_90_post = GetIdxFirstCross(amp*0.9, channel[i], j_10_post, -1);
      var["decaytime"][i] = time[GetTimeIndex(i)][j_10_post] - time[GetTimeIndex(i)][j_10_post];

      /************************************
      // -------------- Do the gaussian fit
      *************************************/
      if( config->channels[i].algorithm.Contains("G") ) {
        float frac = config->channels[i].gaus_fraction;
        unsigned int j_down = GetIdxFirstCross(amp*frac, channel[i], idx_min, -1);
        unsigned int j_up = GetIdxFirstCross(amp*frac, channel[i], idx_min, +1);
        if( j_up - j_down < 4 ) {
          j_up = idx_min + 1;
          j_down = idx_min - 1;
        }

        TF1* fpeak = new TF1("fpeak"+name, "gaus", time[GetTimeIndex(i)][j_down], time[GetTimeIndex(i)][j_up]);

        float ext_sigma = time[GetTimeIndex(i)][j_up] - time[GetTimeIndex(i)][j_down];
        if (amp*frac < -baseline_RMS) ext_sigma *= 0.25;
        fpeak->SetParameter(0, amp * sqrt(2*3.14) * ext_sigma );
        fpeak->SetParameter(1, time[GetTimeIndex(i)][idx_min]);
        fpeak->SetParameter(2, ext_sigma);

        TString opt = "R";
        if ( draw_debug_pulses ) opt += "+";
        else opt += "QN0";
        pulse->Fit("fpeak"+name, opt);

        var["gaus_mean"][i] = fpeak->GetParameter(1);
        var["gaus_sigma"][i] = fpeak->GetParameter(2);
        var["gaus_chi2"][i] = pulse->Chisquare(fpeak, "R");

        delete fpeak;
      }
      /*******************************
      // -------------- Do  linear fit
      ********************************/
      if( config->channels[i].algorithm.Contains("Re") ) {
        unsigned int i_min = GetIdxFirstCross(config->channels[i].re_bounds[0]*amp, channel[i], idx_min, -1);
        unsigned int i_max = GetIdxFirstCross(config->channels[i].re_bounds[1]*amp, channel[i], i_min, +1);
        float t_min = time[GetTimeIndex(i)][i_min];
        float t_max = time[GetTimeIndex(i)][i_max];

        TF1* flinear = new TF1("flinear"+name, "[0]*x+[1]", t_min, t_max);
        flinear->SetLineColor(2);

        TString opt = "R";
        if ( draw_debug_pulses ) opt += "+";
        else opt += "QN0";
        pulse->Fit("flinear"+name, opt);
        Re_slope = flinear->GetParameter(0);
        Re_b     = flinear->GetParameter(1);

        var["linear_RE_risetime"][i] = (0.90*amp-Re_b)/Re_slope - (0.10*amp-Re_b)/Re_slope;
        for ( auto f : config->constant_fraction ) {
          var[Form("linear_RE_%d", (int)(100*f))][i] = (f*amp-Re_b)/Re_slope;
        }

        delete flinear;
      }
      /*************************************
      // -------------- Local polinomial fit
      **************************************/
      if ( config->constant_fraction.size() ) {
        float start_level =  - 3 * baseline_RMS;
        unsigned int j_start =  GetIdxFirstCross( start_level, channel[i], idx_min, -1);

        for(auto f : config->constant_fraction) {
          unsigned int j_st = j_start;
          if ( amp*f > start_level ) {
            if ( amp*f > -baseline_RMS && verbose) {
              if(N_warnings< N_warnings_to_print) {
                N_warnings++;
                cout << Form("[WARNING] ev:%d ch:%d - fraction %.2f below noise RMS", i_evt, i, f) << endl;
              }
              else if (N_warnings_to_print == N_warnings) {
                N_warnings++;
                cout << "[WARNING] Max number of warnings passed. No more warnings will be printed." << endl;;
              }
            }
            j_st =  GetIdxFirstCross( amp*f, channel[i], idx_min, -1);
          }

          unsigned int j_close = GetIdxFirstCross(amp*f, channel[i], j_st, +1);
          if ( fabs(channel[i][j_close-1] - f*amp) < fabs(channel[i][j_close] - f*amp) ) j_close--;

          if( config->channels[i].algorithm.Contains("IL")) {
            unsigned int j_aux = j_close + 1;

            float t1 = time[GetTimeIndex(i)][j_close];
            float v1 = channel[i][j_close];
            float t2 = time[GetTimeIndex(i)][j_aux];
            float v2 = channel[i][j_aux];


            float out = 0;
            if (v1 == v2) {
              out = TMath::Max(t1, t2);
            }
            else {
              out = t1 + (t2 - t1) * (amp*f - v1)/(v2 - v1);
            }

            // cout << Form("%g: %g %g %g %g %g %g", f, t1, v1, t2, v2, amp*f, out) << endl;
            var[Form("IL_%d", (int)(100*f))][i] = out;
          }

          if (config->channels[i].algorithm.Contains("SPL")) {
            TGraph* inv_pulse = new TGraph(6, &(channel[i][j_close-2]), &(time[GetTimeIndex(i)][j_close-2]));
            var[Form("SPL_%d", (int)(100*f))][i] = inv_pulse->Eval(amp*f, 0, "S");
            delete inv_pulse;
          }

          for(auto n : config->channels[i].PL_deg) {
            unsigned int span_j = (int) (min( j_90_pre-j_close , j_close-j_st)/1.5);

            if (j_90_pre - j_10_pre <= 3*n) {
              span_j = max((unsigned int)(n*0.5), span_j);
              span_j = max((unsigned int)1, span_j);
            }
            else {
              span_j = max((unsigned int) n, span_j);
            }

            if( j_close < span_j || j_close + span_j >= NUM_SAMPLES ) {
              cout << Form("[WARNING] evt %d ch %d:  Short span around the closest point. Analytical fit not performed.", i_evt, i) << endl;
              continue;
            }

            float* coeff;
            int N_add = 1;
            if (span_j + N_add + j_close < j_90_pre) {
              N_add++;
            }
            AnalyticalPolinomialSolver( 2*span_j + N_add , &(channel[i][j_close - span_j]), &(time[GetTimeIndex(i)][j_close - span_j]), n, coeff);

            var[Form("LP%d_%d", n, (int)(100*f))][i] = PolyEval(f*amp, coeff, n);

            if(draw_debug_pulses) {
              coeff_poly_fit.push_back(coeff);
              poly_bounds.push_back(pair<int,int>(j_close-span_j, j_close+span_j+N_add-1));
            }
            else delete [] coeff;
          }
        }
      }
      if ( config->constant_threshold.size() && config->channels[i].algorithm.Contains("LP")) {
        float start_level =  - 3 * baseline_RMS;
        unsigned int j_start =  GetIdxFirstCross( start_level, channel[i], idx_min, -1);

        for(auto thr : config->constant_threshold) {
          if (thr < amp ) continue;
          unsigned int j_st = j_start;
          if ( thr > start_level ) {
            if (thr > -baseline_RMS && verbose) {
              if(N_warnings< N_warnings_to_print) {
                N_warnings++;
                cout << Form("[WARNING] ev:%d ch:%d - thr %.2f mV below noise RMS", i_evt, i, thr) << endl;
              }
              else if (N_warnings_to_print == N_warnings) {
                N_warnings++;
                cout << "[WARNING] Max number of warnings passed. No more warnings will be printed." << endl;;
              }
            }
            j_st =  GetIdxFirstCross( thr, channel[i], idx_min, -1);
          }

          unsigned int j_close = GetIdxFirstCross(thr, channel[i], j_st, +1);
          if ( fabs(channel[i][j_close-1] - thr) < fabs(channel[i][j_close] - thr) ) j_close--;

          if( config->channels[i].algorithm.Contains("IL")) {
            unsigned int j_aux = j_close + 1;

            float t1 = time[GetTimeIndex(i)][j_close];
            float v1 = channel[i][j_close];
            float t2 = time[GetTimeIndex(i)][j_aux];
            float v2 = channel[i][j_aux];


            float out = 0;
            if (v1 == v2) {
              out = TMath::Max(t1, t2);
            }
            else {
              out = t1 + (t2 - t1) * (thr - v1)/(v2 - v1);
            }

            var[Form("IL_%dmV", (int)(fabs(thr)))][i] = out;
          }

          if (config->channels[i].algorithm.Contains("SPL")) {
            TGraph* inv_pulse = new TGraph(6, &(channel[i][j_close-2]), &(time[GetTimeIndex(i)][j_close-2]));
            var[Form("SPL_%dmV", (int)(fabs(thr)))][i] = inv_pulse->Eval(thr, 0, "S");
            delete inv_pulse;
          }

          for(auto n : config->channels[i].PL_deg) {
            unsigned int span_j = (int) (min( j_90_pre-j_close , j_close-j_st)/1.5);

            if (j_90_pre - j_10_pre <= 3*n) {
              span_j = max((unsigned int)(n*0.5), span_j);
              span_j = max((unsigned int)1, span_j);
            }
            else {
              span_j = max((unsigned int) n, span_j);
            }

            if( j_close < span_j || j_close + span_j >= NUM_SAMPLES ) {
              if (verbose) {
                cout << Form("[WARNING] evt %d ch %d:  Short span around the closest point. Analytical fit not performed.", i_evt, i) << endl;
                cout << j_close << "  " << span_j << "  " << thr << endl;
              }
              continue;
            }

            float* coeff;
            int N_add = 1;
            if (span_j + N_add + j_close < j_90_pre) {
              N_add++;
            }
            AnalyticalPolinomialSolver( 2*span_j + N_add , &(channel[i][j_close - span_j]), &(time[GetTimeIndex(i)][j_close - span_j]), n, coeff);

            var[Form("LP%d_%dmV", n, (int)(fabs(thr)))][i] = PolyEval(thr, coeff, n);

            if(draw_debug_pulses) {
              coeff_poly_fit.push_back(coeff);
              poly_bounds.push_back(pair<int,int>(j_close-span_j, j_close+span_j+N_add-1));
            }
            else delete [] coeff;
          }
        }
      }

      /*****************************************************
      Get times from Sin(x)/x interpolation (Luciano's code)
      ******************************************************/
      //std::cout << "===============" << i << "================"<< std::endl;
      if( config->channels[i].algorithm.Contains("TOT") )
      {
        //Create Interpolator object
        Interpolator *voltage = new Interpolator;
        voltage->init(NUM_SAMPLES, time[GetTimeIndex(i)][0], time[GetTimeIndex(i)][NUM_SAMPLES-1], channel[i]);

        //compute the signal amplitude from interpolation
        double tStep = (time[GetTimeIndex(i)][NUM_SAMPLES-1] - time[GetTimeIndex(i)][0])/(double)(NUM_SAMPLES-1)*1. ;
        double tmpT = var["t_peak"][i]-2.*var["risetime"][i];
        var["InterpolatedAmp"][i] = 666;
        //std::cout << "====================" << std::endl;
        while ( tmpT < var["t_peak"][i] + 5.0 )
        {
          //cout << "time : " << tmpT << " --> " << voltage->f(tmpT) ;
          if (voltage->f(tmpT) < var["InterpolatedAmp"][i])//find minimum (negative going pulses)
          {
            var["InterpolatedAmp"][i] = voltage->f(tmpT);
            //cout << " !!!MAX!!! ";
          }
          //cout << "\n";
          tmpT += 0.001;
        }

        var["InterpolatedAmp"][i] = fabs(var["InterpolatedAmp"][i]);

        //Constant threshold timestamping
        for (auto thr : config->constant_threshold)
        {
          var[Form("t0_%d", (int)(fabs(thr)))][i] = -666;
          var[Form("t1_%d", (int)(fabs(thr)))][i] = -666;
          if ( var["amp"][i] > fabs(thr) )
          {
            //std::cout << "=====Constant Threshold======" << var["amp"][i] << " " << thr << "========================" << std::endl;
            int retCode = TimeOverThreshold( voltage, thr, var["t_peak"][i]-2.*var["risetime"][i], time[GetTimeIndex(i)][NUM_SAMPLES-1], i, GetTimeIndex(i), var[Form("t0_%d", (int)(fabs(thr)))][i],var[Form("t1_%d", (int)(fabs(thr)))][i]);
            if( retCode == 0 ) var[Form("tot_%d", (int)(fabs(thr)))][i] = var[Form("t1_%d", (int)(fabs(thr)))][i]-var[Form("t0_%d", (int)(fabs(thr)))][i];
          }
        }
        //CFD time stamping
        for (auto thr : config->constant_fraction)
        {
          var[Form("t0CFD_%d", (int)(100*thr))][i] = -666;
          var[Form("t1CFD_%d", (int)(100*thr))][i] = -666;
          var[Form("totCFD_%d", (int)(100*thr))][i] = -666;
          if ( var["amp"][i] > fabs(thr) && var["InterpolatedAmp"][i] > 0 )
          {
            //pulses are negative, so need to multiply the amplitude by -1.0
            double CFDThreshold = -1.0 * thr * var["InterpolatedAmp"][i];
            //std::cout << "===CFD====" << var["amp"][i] << " " << CFDThreshold << "========================" << std::endl;
            int retCode = TimeOverThreshold( voltage, CFDThreshold, var["t_peak"][i]-2.*var["risetime"][i], 200 , i, GetTimeIndex(i), var[Form("t0CFD_%d", (int)(100*thr))][i],var[Form("t1CFD_%d", (int)(100*thr))][i]);
            if( retCode == 0 ) var[Form("totCFD_%d", (int)(100*thr))][i] = var[Form("t1CFD_%d", (int)(100*thr))][i]-var[Form("t0CFD_%d", (int)(100*thr))][i];
          }
        }
      } //end if algorithm.Contains("TOT")
    }


    /*********************************************
    // ===================  Draw plot of the pulse
    **********************************************/
    if(draw_debug_pulses) {

      cout << "========= Event: " << i_evt << " - ch: " << i << endl;

      TCanvas* c =  new TCanvas("c_"+name, "c_"+name, 1600, 600);
      c->Divide(2);

      TLine* line = new TLine();

      // ---------- All range plot
      c->cd(1);
      c->SetGrid();
      // Draw pulse
      pulse->SetMarkerStyle(4);
      pulse->SetMarkerSize(0.5);
      pulse->GetYaxis()->SetTitle("Amplitude [mV]");
      pulse->GetXaxis()->SetTitle("Time [ns]");
      pulse->Draw("APE1");
      // Draw baseline
      line->SetLineWidth(1);
      line->SetLineColor(46);
      line->SetLineStyle(7);
      line->DrawLine(time[GetTimeIndex(i)][0], 0, time[GetTimeIndex(i)][NUM_SAMPLES-1], 0);
      line->SetLineStyle(1);
      line->DrawLine(time[GetTimeIndex(i)][bl_st_idx], 0, time[GetTimeIndex(i)][bl_st_idx+bl_lenght], 0);
      line->SetLineColor(47);
      line->DrawLine(time[GetTimeIndex(i)][0], var["baseline_RMS"][i], time[GetTimeIndex(i)][NUM_SAMPLES-1], var["baseline_RMS"][i]);
      line->DrawLine(time[GetTimeIndex(i)][0], -var["baseline_RMS"][i], time[GetTimeIndex(i)][NUM_SAMPLES-1], -var["baseline_RMS"][i]);

      // Draw peak
      line->SetLineColor(8);
      line->SetLineStyle(4);
      line->DrawLine(time[GetTimeIndex(i)][0], amp, var["t_peak"][i], amp);
      line->DrawLine(var["t_peak"][i], 0, var["t_peak"][i], amp);

      // Draw 10% and 90% lines;
      TLine* line_lvs = new TLine();
      line_lvs->SetLineWidth(1);
      line_lvs->SetLineColor(4);
      line_lvs->DrawLine(time[GetTimeIndex(i)][0], 0.1*amp, time[GetTimeIndex(i)][NUM_SAMPLES-1], 0.1*amp);
      line_lvs->DrawLine(time[GetTimeIndex(i)][0], 0.9*amp, time[GetTimeIndex(i)][NUM_SAMPLES-1], 0.9*amp);
      // Draw constant fractions lines
      line_lvs->SetLineColor(38);
      line_lvs->SetLineStyle(10);
      for(auto f : config->constant_fraction) {
        line_lvs->DrawLine(time[GetTimeIndex(i)][0], f*amp, time[GetTimeIndex(i)][NUM_SAMPLES-1], f*amp);
      }
      // Draw constant threshold lines
      line_lvs->SetLineColor(28);
      for(auto thr : config->constant_threshold) {
        line_lvs->DrawLine(time[GetTimeIndex(i)][0], thr, time[GetTimeIndex(i)][NUM_SAMPLES-1], thr);
      }


      // Draw integral area
      int N_tot_integral = j_area_post-j_area_pre;
      if( N_tot_integral > 0 ) {
        vector<float> aux_time = {time[GetTimeIndex(i)][j_area_pre]};
        vector<float> aux_volt = {0};
        for(unsigned int j = j_area_pre; j <= j_area_post; j++) {
          aux_time.push_back(time[GetTimeIndex(i)][j]);
          aux_volt.push_back(channel[i][j]);
        }
        aux_time.push_back(time[GetTimeIndex(i)][j_area_post]);
        aux_volt.push_back(0);
        TGraph * integral_pulse = new TGraph(aux_time.size(), &(aux_time[0]), &(aux_volt[0]));
        integral_pulse->SetFillColor(40);
        integral_pulse->SetFillStyle(3144);
        integral_pulse->Draw("FC");
        TText* t_int = new TText(var["t_peak"][i], amp, Form("Int = %1.2e (%1.2e) pC", var["integral"][i], var["intfull"][i]));
        t_int->SetTextAlign(kHAlignLeft+kVAlignBottom);
        t_int->Draw();

        // Draw 90% and 10% pre and post points
        TGraph* gr_pre_post = new TGraph(4);
        gr_pre_post->SetPoint(0, time[GetTimeIndex(i)][j_10_pre], channel[i][j_10_pre]);
        gr_pre_post->SetPoint(1, time[GetTimeIndex(i)][j_90_pre], channel[i][j_90_pre]);
        gr_pre_post->SetPoint(2, time[GetTimeIndex(i)][j_10_post], channel[i][j_10_post]);
        gr_pre_post->SetPoint(3, time[GetTimeIndex(i)][j_90_post], channel[i][j_90_post]);
        gr_pre_post->SetMarkerColor(4);
        gr_pre_post->Draw("P*");


        // ---------- Rising edge only inverted!! -----
        c->cd(2);
        c->SetGrid();

        if( config->channels[i].algorithm.Contains("Re") ) {
          unsigned int i_min = GetIdxFirstCross(config->channels[i].re_bounds[0]*amp, channel[i], idx_min, -1);
          unsigned int i_max = GetIdxFirstCross(config->channels[i].re_bounds[1]*amp, channel[i], i_min, +1);
          float y[2], x[2];
          x[0] = channel[i][i_min];
          x[1] = channel[i][i_max];
          y[0] = (channel[i][i_min] - Re_b)/Re_slope;
          y[1] = (channel[i][i_max] - Re_b)/Re_slope;

          TGraph* gr_Re = new TGraph(2, x, y);

          gr_Re->SetLineColor(46);
          gr_Re->SetLineWidth(1);
          gr_Re->SetLineStyle(7);
          gr_Re->Draw("CP");
        }

        unsigned int j_begin = j_10_pre - 6;
        unsigned int j_span = j_90_pre - j_10_pre + 10;
        TGraphErrors* inv_pulse = new TGraphErrors(j_span, &(channel[i][j_begin]), &(time[GetTimeIndex(i)][j_begin]), yerr);
        inv_pulse->SetNameTitle("g_inv"+name, "g_inv"+name);
        inv_pulse->SetMarkerStyle(5);
        inv_pulse->GetXaxis()->SetTitle("Amplitude [mV]");
        inv_pulse->GetYaxis()->SetTitle("Time [ns]");
        inv_pulse->Draw("APE1");

        vector<float> t_WS;
        vector<float> c_WS;
        float overstep = 6;
        for(unsigned int jj = j_begin; jj < j_begin + j_span; jj++) {
          for(unsigned int kk = 0; kk < overstep; kk++) {
            float tt = time[GetTimeIndex(i)][jj] + (time[GetTimeIndex(i)][jj+1] - time[GetTimeIndex(i)][jj]) * kk/overstep;
            float cc = WSInterp(tt, NUM_SAMPLES, time[GetTimeIndex(i)], channel[i]);

            t_WS.push_back(tt);
            c_WS.push_back(cc);
          }
        }
        TGraph* inv_pulse_WS = new TGraph(t_WS.size(), &(c_WS[0]), &(t_WS[0]));
        inv_pulse_WS->SetMarkerStyle(7);
        inv_pulse_WS->SetMarkerColor(2);
        // inv_pulse_WS->Draw("P");

        TGraph* gr_inv_pre_post = new TGraph(2);
        gr_inv_pre_post->SetPoint(0, channel[i][j_10_pre], time[GetTimeIndex(i)][j_10_pre]);
        gr_inv_pre_post->SetPoint(1, channel[i][j_90_pre], time[GetTimeIndex(i)][j_90_pre]);
        gr_inv_pre_post->SetMarkerColor(4);
        gr_inv_pre_post->Draw("P*");

        // -------------- If exist, draw local polinomial fit
        unsigned int count = 0;
        vector<int> frac_colors = {2, 6, 8, 5, 40, 46, 4, 9, 12};
        while(frac_colors.size() < config->constant_fraction.size() + config->constant_threshold.size()) {
          frac_colors.push_back(2);
        }

        for( unsigned int kk = 0; kk < config->constant_fraction.size(); kk++) {
          float f = config->constant_fraction[kk];
          line_lvs->SetLineColor(frac_colors[kk]);
          line_lvs->DrawLine(amp*f, time[GetTimeIndex(i)][j_begin], amp*f, time[GetTimeIndex(i)][j_90_pre + 3]);
          for(auto n : config->channels[i].PL_deg) {
            vector<float> polyval;
            for(unsigned int j = poly_bounds[count].first; j <= poly_bounds[count].second; j++) {
              polyval.push_back(PolyEval(channel[i][j], coeff_poly_fit[count], n));
            }

            TGraph* g_poly = new TGraph(polyval.size(), &(channel[i][poly_bounds[count].first]), &(polyval[0]) );
            g_poly->SetLineColor(frac_colors[kk]);
            g_poly->SetLineWidth(2);
            g_poly->SetLineStyle(7);
            g_poly->Draw("C");

            count++;
          }
        }

        for( unsigned int kk = 0; kk < config->constant_threshold.size(); kk++) {
          float thr = config->constant_threshold[kk];
          if (thr < amp ) continue;
          line_lvs->SetLineColor(frac_colors[kk + config->constant_fraction.size()]);
          line_lvs->DrawLine(thr, time[GetTimeIndex(i)][j_begin], thr, time[GetTimeIndex(i)][j_90_pre + 3]);
          for(auto n : config->channels[i].PL_deg) {
            vector<float> polyval;
            for(unsigned int j = poly_bounds[count].first; j <= poly_bounds[count].second; j++) {
              polyval.push_back(PolyEval(channel[i][j], coeff_poly_fit[count], n));
            }

            TGraph* g_poly = new TGraph(polyval.size(), &(channel[i][poly_bounds[count].first]), &(polyval[0]) );
            g_poly->SetLineColor(frac_colors[kk + config->constant_fraction.size()]);
            g_poly->SetLineWidth(2);
            g_poly->SetLineStyle(7);
            g_poly->Draw("C");

            count++;
          }
        }

      }

      c->SetGrid();
      c->SaveAs("./pulses_imgs/"+name+img_format);
      delete c;
    }


    if ( NUM_F_SAMPLES > 0 )
    {
      const unsigned int n_samples_interpolation = 5000;
      float time_interpolation[n_samples_interpolation];
      float channel_interpolation[n_samples_interpolation];
      Interpolator voltage;
      voltage.init(NUM_SAMPLES, time[GetTimeIndex(i)][0], time[GetTimeIndex(i)][NUM_SAMPLES-1], channel[i]);
      float deltaT = (time[GetTimeIndex(i)][NUM_SAMPLES-1]-time[GetTimeIndex(i)][0])/float(n_samples_interpolation);
      for (unsigned int is = 0; is < n_samples_interpolation; is++)
      {
        time_interpolation[is] = float(is)*deltaT;
        channel_interpolation[is] = voltage.f(float(is)*deltaT);
      }

      /***************************************
      Fourier Transform
      ****************************************/

      for (unsigned int ifq = 0; ifq < NUM_F_SAMPLES; ifq++ )
      {
        //channel_spectrum[i][ifq] = FrequencySpectrum( frequency[ifq], var["t_peak"][i]-3.,
         //var["t_peak"][i]+3., i, GetTimeIndex(i));
         channel_spectrum[i][ifq] = FrequencySpectrum( frequency[ifq], var["t_peak"][i]-3., var["t_peak"][i]+3.,
          n_samples_interpolation, channel_interpolation, time_interpolation);
      }
    }

    delete [] yerr;
    delete pulse;
  }
}

/*
*****************************************
Loop Over All Events and Check Corruption
*****************************************
*/
void DatAnalyzer::RunEventsLoop() {
    std::cout << "before Events loop started" << std::endl;
    InitLoop();

    unsigned int evt_progress_print_rate = verbose ? 100 : 1000;
    evt_progress_print_rate = 1;

    std::cout << "Events loop started" << std::endl;
    unsigned int N_written_evts = 0;
    if ( bin_file != NULL )
    {
      for( i_evt = 0; !feof(bin_file) && (N_evts==0 || i_evt<N_evts); i_evt++){
      	if (i_evt % 100 == 0) cout << "Processing Event " << i_evt << "\n";
        int corruption = GetChannelsMeasurement();
        if(corruption == -1) break;
        else if (corruption == 1) {
          cout << "Corruption skip SUCCEDED!" << endl;
          cout << "Not analyzing current loaded evt" << endl;
        }

        if( i_evt >= start_evt ) {
          if (corruption == 0) Analyze();
          N_written_evts++;
          tree->Fill();
          if(N_written_evts%evt_progress_print_rate == 0) {
            //cout << "!!!!!!!!!!!!!!!!! Event : " << N_written_evts << endl;
          }
        }
      }
    }
    else if ( tree_in != NULL )
    {
      int n_evt_tree = tree_in->GetEntries();
      //std::cout << "NNNN: " << n_evt_tree << std::endl;
      for(int i_aux = start_evt; i_aux < n_evt_tree && (N_evts==0 || i_aux<N_evts); i_aux++){
        if (i_aux % 100 == 0) cout << "Processing Event " << i_aux << "\n";

        GetChannelsMeasurement( i_aux );
        Analyze();

        N_written_evts++;
        tree->Fill();
        if(N_written_evts%evt_progress_print_rate == 0) {
          //cout << "!!!!!!!!!!!!!!!!! Event : " << N_written_evts << endl;
        }
      }
    }

    if ( bin_file != NULL ) fclose(bin_file);
    cout << "\nLoaded total of " << tree->GetEntries() << " (" << i_evt << ") events\n";


    if(N_evt_expected>0 && N_evt_expected!=i_evt && N_evts == 0) {
      cout << endl;
      cout << "====================== WARNING =====================" << endl;
      cout << "|    Number of events not matching expectations    |" << endl;
      cout << "          " << N_evt_expected << "  !=  " << i_evt << endl;
      cout << "====================================================" << endl;
      cout << endl;
    }
    file->Write();
    cout << "\nWritten total of " << N_written_evts << " events\n";
}


TString DatAnalyzer::ParseCommandLine( int argc, char* argv[], TString opt )
{
  TString out = "";
  for (int i = 1; i < argc && out==""; i++ ) {
    TString tmp( argv[i] );
    if ( tmp.Contains("--"+opt) ) {
      if(tmp.Contains("=")) {
        out = tmp(tmp.First("=")+1, tmp.Length());
      }
      else {
        out = "true";
      }
    }
  }
  return out;
}

void DatAnalyzer::GetCommandLineArgs(int argc, char **argv) {
  cout << endl << "-------------- TimingDAQ (Dat2Root) --------------" << endl;

  input_file_path = ParseCommandLine( argc, argv, "input_file" );
  ifstream in_file(input_file_path.Data());
  if (!in_file || input_file_path == "" || !(input_file_path.EndsWith(".dat") || input_file_path.EndsWith(".root")) ){
    cerr << "[ERROR]: please provide a valid input file. Use: --input_file=<your_input_file_path>.dat or  --input_file=<your_input_file_path>.root" << endl;
    exit(0);
  }
  else if (verbose)
  {
    cout << "Input file: " << input_file_path.Data() << endl;
  }

  TString aux;
  aux = ParseCommandLine( argc, argv, "verbose" );
  aux.ToLower();
  if(aux == "true") verbose = true;

  aux = ParseCommandLine( argc, argv, "config" );
  if(aux == ""){
    cerr << "[ERROR]: Missing config file" << endl;
    exit(0);
  }
  if(verbose) {cout << "Config file: " << aux.Data() << endl;}
  config = new Configuration(aux.Data(), verbose);
  if ( !config->isValid() ) {
    cerr << "\nFailed to load channel information from config " << aux.Data() << endl;
    exit(0);
  }

  // -------- Non compulsory command line arguments
  output_file_path = ParseCommandLine( argc, argv, "output_file" );
  if ( output_file_path == "" ){
    output_file_path = input_file_path;

    if ( output_file_path.EndsWith(".dat") )
    {
      output_file_path.ReplaceAll(".dat", "_converted.root");
    }
    else if ( output_file_path.EndsWith(".root") )
    {
      output_file_path.ReplaceAll(".root", "_converted.root");
    }
    std::cout << "=====" << output_file_path << std::endl;
  }
  else if (!output_file_path.EndsWith(".root")) output_file_path += ".root";
  if (verbose) {std::cout << "Output file: " << output_file_path.Data() << std::endl;}


  aux = ParseCommandLine( argc, argv, "N_evts" );
  long int N_tmp = aux.Atoi();
  if(N_tmp<0) {
    cout << "[ERROR]: Number of events has to be positive or 0. If 0 the whole file will be analyzed." << endl;
    exit(0);
  }
  N_evts = N_tmp;
  if ( verbose ) {
    cout << "Number of events: " << flush;
    if(N_evts == 0){ cout << "Not specified." << endl;}
    else{ cout << N_evts << endl;}
  }

  aux = ParseCommandLine( argc, argv, "start_evt" );
  if(aux != "") {
    start_evt = aux.Atoi();
    cout << "[INFO] Starting from event: " << start_evt << endl;
  }

  aux = ParseCommandLine( argc, argv, "N_evt_expected" );
  N_evt_expected = aux.Atoi();
  if(N_evt_expected>0) {
    cout << "[INFO] Number of expected events: " << flush;
    cout << N_evt_expected << endl;
  }

  aux = ParseCommandLine( argc, argv, "save_raw" );
  aux.ToLower();
  if(aux == "true")
  {
    save_raw = true;
    std::cout << "save raw" << std::endl;
  }

  aux = ParseCommandLine( argc, argv, "save_meas" );
  aux.ToLower();
  if(aux == "true") save_meas = true;

  aux = ParseCommandLine( argc, argv, "draw_debug_pulses" );
  aux.ToLower();
  if(aux != "false" && aux != "") {
    if(aux != "true") img_format = aux;
    cout << "[INFO]: Saving debug pulses in " << img_format.Data() << endl;
    draw_debug_pulses =  true;

    struct stat info;
    if( stat( "./pulses_imgs", &info ) != 0 ) {
      cout << "[ERROR]: Cannot access ./pulses_imgs" << endl;
      cout << "--------> Please create it and rerun." << endl;
      exit(0);
    }
  }
}

void DatAnalyzer::InitLoop() {
    std::cout << "Initializing input file reader and output tree" << std::endl;
    file = new TFile(output_file_path.Data(), "RECREATE");
    ifstream out_file(output_file_path.Data());
    if (!out_file){
      cerr << "[ERROR]: Cannot create output file: " << output_file_path.Data() << endl;
      exit(0);
    }
    tree = new TTree("pulse", "Digitized waveforms");
    tree->Branch("i_evt", &i_evt, "i_evt/i");

    std::cout << "Initializing input root file" << std::endl;
    if ( input_file_path.EndsWith(".root") )//place holder for input file in the future.
    {
      //file_in = new TFile("/Users/cmorgoth/git/TimingDAQ/LGADSimulation_SNR20_ShapingTime1.root","READ");
      file_in = new TFile(input_file_path,"READ");
      tree_in = (TTree*)file_in->Get("pulse");
    }

    /*
    ************************
    SAVE PULSHAPES
    ************************
    */
    if(save_meas){
      tree->Branch("channel", &(channel[0][0]), Form("channel[%d][%d]/F", NUM_CHANNELS, NUM_SAMPLES));
      tree->Branch("time", &(time[0][0]), Form("time[%d][%d]/F", NUM_TIMES, NUM_SAMPLES));
      if ( NUM_F_SAMPLES > 0 )
      {
        tree->Branch("channel_spectrum", &(channel_spectrum[0][0]), Form("channel_spectrum[%d][%d]/F", NUM_CHANNELS, NUM_F_SAMPLES));
        tree->Branch("frequency", &(frequency[0]), Form("frequency[%d]/F", NUM_F_SAMPLES));
      }
    }

    /*
    **********************************
    Obtain Algorithms from config file
    **********************************
    */
    bool at_least_1_gaus_fit = false;
    bool at_least_1_rising_edge = false;
    int at_least_1_LP[3] = {false};
    bool at_least_1_IL = false;
    bool at_least_1_SPL = false;
    bool at_least_1_TOT = false;
    for(auto c : config->channels) {
      if( c.second.algorithm.Contains("G")) at_least_1_gaus_fit = true;
      if( c.second.algorithm.Contains("Re")) at_least_1_rising_edge = true;
      if( c.second.algorithm.Contains("LP1")) at_least_1_LP[0] = true;
      if( c.second.algorithm.Contains("LP2")) at_least_1_LP[1] = true;
      if( c.second.algorithm.Contains("LP3")) at_least_1_LP[2] = true;
      if( c.second.algorithm.Contains("IL")) at_least_1_IL = true;
      if( c.second.algorithm.Contains("SPL")) at_least_1_SPL = true;
      if( c.second.algorithm.Contains("TOT")) at_least_1_TOT = true;
    }

    /*
    **************************************************************
    Set Constant Fractions and Thresholds for different algorithms
    **************************************************************
    */
    if( at_least_1_IL) {
      for (auto f : config->constant_fraction) {
        var_names.push_back(Form("IL_%d", (int)(100*f)));
      }
      for (auto thr : config->constant_threshold) {
        var_names.push_back(Form("IL_%dmV", (int)(fabs(thr))));
      }
    }

    if( at_least_1_SPL) {
      for (auto f : config->constant_fraction) {
        var_names.push_back(Form("SPL_%d", (int)(100*f)));
      }
      for (auto thr : config->constant_threshold) {
        var_names.push_back(Form("SPL_%dmV", (int)(fabs(thr))));
      }
    }

    for(unsigned int i = 0; i< 3; i++) {
      if(at_least_1_LP[i]) {
        for (auto f : config->constant_fraction) {
          var_names.push_back(Form("LP%d_%d", i+1, (int)(100*f)));
        }
        for (auto thr : config->constant_threshold) {
          var_names.push_back(Form("LP%d_%dmV", i+1, (int)(fabs(thr))));
        }
      }
    }
    /*
    ************
    Gaussian Fit
    ************
    */
    if( at_least_1_gaus_fit ) {
      var_names.push_back("gaus_mean");
      var_names.push_back("gaus_sigma");
      var_names.push_back("gaus_chi2");
    }
    /*
    **********************
    Linear Rising Edge Fit(good old stuff!)
    **********************
    */
    if( at_least_1_rising_edge ) {
      var_names.push_back("linear_RE_risetime");
      for (auto f : config->constant_fraction) {
        var_names.push_back(Form("linear_RE_%d", (int)(100*f)));
      }
    }

    /*******************************************
    TOT Variables
    ********************************************/
    if( at_least_1_TOT ) {
      var_names.push_back("InterpolatedAmp");
      for (auto thr : config->constant_threshold)
      {
        var_names.push_back(Form("t0_%d", (int)(fabs(thr))));
        var_names.push_back(Form("t1_%d", (int)(fabs(thr))));
        var_names.push_back(Form("tot_%d", (int)(fabs(thr))));
      }
      for (auto thr : config->constant_fraction)
      {
        var_names.push_back(Form("t0CFD_%d", (int)(100*fabs(thr))));
        var_names.push_back(Form("t1CFD_%d", (int)(100*fabs(thr))));
        var_names.push_back(Form("totCFD_%d", (int)(100*fabs(thr))));
      }
    }

    /*
    *******************************************************
    // Create the tree beanches an the associated variables
    *******************************************************
    */
    if ( verbose ) { cout << "Initializing all tree variables" << endl; }
    if (save_meas && verbose) {
      cout << "   channel\n   time" << endl;
    }
    for(TString n : var_names){
      var[n] = new float[NUM_CHANNELS];
      tree->Branch(n, &(var[n][0]), n+Form("[%d]/F", NUM_CHANNELS));
      if( verbose ) { cout << "   " << n.Data() << endl; }
    }

    for(unsigned int i = 0; i < NUM_CHANNELS; i++) ResetVar(i);
    // Initialize the input file stream
    if ( input_file_path.EndsWith(".dat") )
    {
      bin_file = fopen( input_file_path.Data(), "r" );
    }
};

void DatAnalyzer::ResetVar(unsigned int n_ch) {
  for(auto n: var_names) {
    var[n][n_ch] = DEFAULT_FOR_EMPTY_CH;
  }
}

void DatAnalyzer::ResetAnalysisVariables() {
  for(unsigned int i=0; i<NUM_CHANNELS; i++) {
    for(unsigned int j=0; j<NUM_SAMPLES; j++) {
      channel[i][j] = 0;
      if(i < NUM_TIMES) time[i][j] = 0;
    }
  }
}

float DatAnalyzer::GetPulseIntegral(float *a, float *t, unsigned int i_st, unsigned int i_stop) //returns charge in pC asssuming 50 Ohm termination
{
  //Simpson's Rule for equaled space with Cartwright correction for unequaled space
  float integral = 0.;
  for (unsigned int i=i_st; i < i_stop-2 ; i+=2) {
    float aux = ( 2-(t[i+2]-t[i+1])/(t[i+1]-t[i]) ) * a[i];
    aux += (t[i+2]-t[i])*(t[i+2]-t[i])/((t[i+2]-t[i+1])*(t[i+1]-t[i])) * a[i+1];
    aux += ( 2-(t[i+1]-t[i])/(t[i+2]-t[i+1]) ) * a[i+2];

    integral += ( (t[i+2]-t[i]) / 6.0 ) * aux;
  }

  integral *= 1e-9 * 1e-3 * (1.0/50.0) * 1e12; //in units of pC, for 50 [Ohms] termination
  return integral;
}

unsigned int DatAnalyzer::GetIdxClosest(float value, float* v, unsigned int i_st, int direction) {
  unsigned int idx_end = direction>0 ? NUM_SAMPLES-1 : 0;

  unsigned int i = i_st;
  unsigned int i_min = i_st;
  float min_distance = fabs(value - v[i]);

  while( i != idx_end ) {
    i += direction;
    float d = fabs(value - v[i]);
    if( d < min_distance ) {
      min_distance = d;
      i_min = i;
    }
  }

  return i_min;
}

unsigned int DatAnalyzer::GetIdxFirstCross(float value, float* v, unsigned int i_st, int direction) {
  unsigned int idx_end = direction>0 ? NUM_SAMPLES-1 : 0;

  bool rising = value > v[i_st]? true : false;

  unsigned int i = i_st;

  while( i != idx_end ) {
    if(rising && v[i] > value) break;
    else if( !rising && v[i] < value) break;
    i += direction;
  }

  return i;
}

void DatAnalyzer::AnalyticalPolinomialSolver(unsigned int Np, float* in_x, float* in_y, unsigned int deg, float* &out_coeff, float* err) {
  if(deg <= 0 || deg>3) { cout << "[ERROR]: You don't need AnalyticalPolinomialSolver for this" << endl; exit(0);}

  TVectorF x, x2, x3;
  x.Use(Np, in_x);

  TVectorF y;
  y.Use(Np, in_y);

  TMatrixF A(Np, deg+1);

  TMatrixFColumn(A, 0) = 1.;
  TMatrixFColumn(A, 1) = x;

  float *in_x2 = new float[Np];
  float *in_x3 = new float[Np];
  if( deg >= 2 ) {
    for(unsigned int i = 0; i < Np; i++) in_x2[i] = in_x[i]*in_x[i];
    x2.Use(Np, in_x2);
    TMatrixFColumn(A, 2) = x2;
  }
  if( deg >= 3 ) {
    for(unsigned int i = 0; i < Np; i++) in_x3[i] = in_x2[i] * in_x[i];
    x3.Use(Np, in_x3);
    TMatrixFColumn(A, 3) = x3;
  }

  const TVectorD c_norm = NormalEqn(A,y);
  // // When/if error will be implemented
  // if (err != 0) {
  //   TVectorF e;
  //   e.Use(Np, err);
  //   c_norm = NormalEqn(A,y,e);
  // }
  // else c_norm = NormalEqn(A,y);


  out_coeff = new float[deg+1];
  for(unsigned int i = 0; i<= deg; i++) {
    out_coeff[i] = c_norm[i];
  }


  delete [] in_x2;
  delete [] in_x3;
  return;
}

float DatAnalyzer::PolyEval(float x, float* coeff, unsigned int deg) {
  float out = coeff[0] + x*coeff[1];
  for(unsigned int i=2; i<=deg; i++) {
    out += coeff[i]*pow(x, i);
  }
  return out;
}

float DatAnalyzer::WSInterp(float t, int N, float* tn, float* cn) {
  float out = 0;
  float dt = (tn[0] - tn[N-1])/N;
  for(unsigned i = 0; i < N; i++) {
    float x = (t - tn[i])/dt;
    out += cn[i] * sin(3.14159265358 * x) / (3.14159265358 * x);
  }
  return out;
};


int DatAnalyzer::TimeOverThreshold(Interpolator *voltage, double tThresh, double tMin, double tMax, int ich, int t_index, float& time1, float& time2)
{
  // 	Interpolator voltage;
  // voltage->init(NUM_SAMPLES, time[t_index][0], time[t_index][NUM_SAMPLES-1], channel[ich]);
  double tStep = (time[t_index][NUM_SAMPLES-1] - time[t_index][0])/(double)(NUM_SAMPLES-1)/1. ;
  double tStepInit = tStep;
	double t = tMin;
	unsigned nIterations = 0;
	while( nIterations <= 1000)
	{
		//std::cout << "tStep: " << tStep << std::endl;
	  //std::cout << "t: " << t << " " << voltage->f(t) << " " << channel[ich][0] << " " <<  tThresh << std::endl;
		while((voltage->f(t) - tThresh)*tStep > 0.)//assuming only negative pulses
		{
			//std::cout << nIterations << " t: " << t  << " " << voltage->f(t) << " " << tThresh << std::endl;
			if(t<tMin) return -1;
			if(t>tMax) return -2;
			t += tStep;
		}
		//if( fabs(tStep) < 1e-5 && fabs(voltage->f(t) - tThresh) < 1e-5 ) break;
    if( fabs(tStep) < 5e-4 ) break;
		tStep =- tStep/2.;
		nIterations++;
	}

	if(nIterations == 1000) return -3;//iterations reached maximum
	time1 = t;

  /*
    std::cout << "===================================" << std::endl;
  	std::cout << "time1: " << time1 << " f(t) = " << voltage->f(time1) << std::endl;
  	std::cout << "===================================" << std::endl;
  */
	tStep = tStepInit*10.;
	t += tStep;

	nIterations = 0;
	while( nIterations <= 1000)
	{
		//std::cout << "t: " << t << " " << voltage->f(t) << " " << channel[ich][0] << " " <<  tThresh << std::endl;
		while( (voltage->f(t) - tThresh)*tStep < 0 )
		{
			//std::cout << nIterations << " t: " << t  << " " << voltage->f(t) << " " << tThresh << std::endl;
			if(t<tMin) return -4;
			if(t>tMax)
      {
        //std::cout << "reached t-max" << std::endl;
        return -5;
      }
			t += tStep;
		}
		if( fabs(tStep) < 5e-4 ) break;
		tStep =- tStep/2.;
		nIterations++;
	}

	if(nIterations == 1000) return -6;//iterations reached maximum
	time2 = t;
  /*
  	std::cout << "===================================" << std::endl;
  	std::cout << "time2: " << time2 << " f(t) = " << voltage->f(time2) << std::endl;
  	std::cout << "===================================" << std::endl;
  */
	return 0;

};

float DatAnalyzer::FrequencySpectrum(double freq, double tMin, double tMax, int ich, int t_index){

	const int range = 0; // extension of samples to be used beyond [tMin, tMax]
	double deltaT = (time[t_index][NUM_SAMPLES - 1] - time[t_index][0])/(double)(NUM_SAMPLES - 1); // sampling time interval
	double fCut = 0.5/deltaT; // cut frequency = 0.5 * sampling frequency from WST
	int n_min = floor(tMin/deltaT) - range; // first sample to use
	int n_max = ceil(tMax/deltaT) + range; // last sample to use
	n_min = std::max(n_min,0); // check low limit
	n_max = std::min(n_max, (int)NUM_SAMPLES - 1); // check high limit
	int n_0 = (n_min + n_max)/2;

	TComplex s(0.,0.); // Fourier transform at freq
	TComplex I(0.,1.); // i

	for(int n = n_min; n <= n_max; n++)
	{
		s += deltaT*(double)channel[ich][n]*TComplex::Exp(-I*(2.*TMath::Pi()*freq*(n-n_0)*deltaT));//maybe don't need n_0 here, I think it will just add a phase to the fourier transform
	}
  return s.Rho();
};

float DatAnalyzer::FrequencySpectrum(double freq, double tMin, double tMax, unsigned int n_samples, float* my_channel, float* my_time)
{
  const int range = 0; // extension of samples to be used beyond [tMin, tMax]
	double deltaT = (my_time[n_samples - 1] - my_time[0])/(double)(n_samples - 1); // sampling time interval
	double fCut = 0.5/deltaT; // cut frequency = 0.5 * sampling frequency from WST
	int n_min = floor(tMin/deltaT) - range; // first sample to use
	int n_max = ceil(tMax/deltaT) + range; // last sample to use
	n_min = std::max(n_min,0); // check low limit
	n_max = std::min(n_max, (int)(n_samples - 1)); // check high limit
	int n_0 = (n_min + n_max)/2;

	TComplex s(0.,0.); // Fourier transform at freq
	TComplex I(0.,1.); // i

	for(int n = n_min; n <= n_max; n++)
	{
    s += deltaT*(double)my_channel[n]*TComplex::Exp(-I*(2.*TMath::Pi()*freq*(n-n_0)*deltaT));//maybe don't need n_0 here, I think it will just add a phase to the fourier transform
	}
  return s.Rho();
};
