#include <TFileCollection.h>
#include <TF1.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TMath.h>
 


double comparator(const std::vector<double>& time, const std::vector<double>& data, const double threshold, const double hysteresis, double& timeOverThreshold, int& numberOfpeaks, int& i_th, const bool chooseFirst=false, const int startFrom=0) {
  numberOfpeaks=0;
  i_th=-1;
  int i_peakStart=0; 
  double firstCrossing_tmp;
  double firstCrossing=-1;
  double secondCrossing;
  bool above=false;
  bool lockForHysteresis=false;
  TGraph rising, falling;
  int spline_n=2;
  
  std::vector<double>::const_iterator max = std::max_element(data.begin(), data.end());
  int i_max = max - data.begin();
  for (int i=startFrom+spline_n; i<data.size()-spline_n; ++i) {
//     std::cout<<i<<std::endl;
    // Look for first edge
    if ( !above && !lockForHysteresis && data.at(i)>threshold ) {
//       std::cout<<i<<"\t"<<"!above && !lockForHysteresis && data.at(i)>threshold"<<std::endl;
      firstCrossing_tmp = time.at(i);
      i_peakStart = i;
      above = true;
      lockForHysteresis=true;
    }
    // Lock until above threshold+hysteresis
    if ( above && lockForHysteresis && data.at(i)>threshold+hysteresis) {
//       std::cout<<i<<"\t"<<"above && lockForHysteresis && data.at(i)>threshold+hysteresis"<<std::endl;
      lockForHysteresis=false;
    }
    // Look for second edge
    if ( above && !lockForHysteresis && data.at(i)<threshold ) {
//       std::cout<<i<<"\t"<<"above && !lockForHysteresis && data.at(i)<threshold"<<std::endl;
      if ( !chooseFirst || numberOfpeaks==0  ) {
        for (int j=0; j<spline_n; ++j) {
          falling.SetPoint(j,data.at(i+j-spline_n/2),time.at(i+j-spline_n/2));
        }
        secondCrossing = falling.Eval(threshold,NULL,"");
        i_th=i_peakStart;
      } 
      ++numberOfpeaks;
      above = false;
    }
    
    if (above && i>=data.size()-spline_n-2) {
//       std::cout<<i<<"\t"<<"above && !lockForHysteresis && i>data.size()-spline_n-1"<<std::endl;
        if ( numberOfpeaks==0  ) secondCrossing = time.at(i);
        ++numberOfpeaks;
        i_th=i_peakStart;
        above = false;
        break;
      }
    if ( above && lockForHysteresis && data.at(i)<threshold &&  time.at(i)-firstCrossing_tmp>1e-9 ) {
//       std::cout<<i<<"\t"<<"above && lockForHysteresis && data.at(i)<threshold &&  time.at(i)-firstCrossing_tmp>10e-9"<<std::endl;
      above = false;
      lockForHysteresis=false;
      //       std::cout<<"Unlock hysteresys"<<std::endl;
    }
  }
  if (above) std::cout<<"Problem: finished above!"<<std::endl;
  //interpolation (linear if spline_n=2)
  if (numberOfpeaks==1 || (chooseFirst && numberOfpeaks>0)) {
    for (int j=0; j<spline_n; ++j) {
      rising.SetPoint(j,data.at(i_th+j-spline_n/2),time.at(i_th+j-spline_n/2));
    }
    firstCrossing = rising.Eval(threshold,NULL,"S");   
  }
  timeOverThreshold = secondCrossing-firstCrossing;
  if (timeOverThreshold<0) timeOverThreshold=-1;
  return firstCrossing;
}

struct AlgorithmParameters{
  double cfdRatio;
  int polarity;
  double threshold_used;
  double threshold_ch0;
  double threshold_ch1;
  int counter;
  double rangeMin_ch0;
  double rangeMax_ch0;
  double rangeMin_ch1;
  double rangeMax_ch1;
  double maximum;
  double maximum2;
  bool plot;
  double sigma;
  double scaleFactor;
  
  double hysteresis;
  double timeOverThreshold;
  int numberOfpeaks;
  
  double baseline_n;
  double baseline;
  double baseline_rms;
  double risetime;
  std::vector<double> gauss_values_d0;
  std::vector<double> gauss_values_d1;
  
  double referenceTime;
  double thresholdTime;
  
  int found;
  int rejectedCounter;
  int detectorNumber;
  
  
  
  AlgorithmParameters(const double cfdRatio, const double threshold_ch0=.0, const double threshold_ch1=.0, const double sigma=0, const double hysteresis=1e-3, const double minCh0=-10., const double maxCh0=10., const double minCh1=-10., const double maxCh1=10., const double baseline_n=0.15): cfdRatio(cfdRatio), polarity(polarity), threshold_ch0(threshold_ch0), threshold_ch1(threshold_ch1), counter(0), rangeMin_ch0(minCh0), rangeMax_ch0(maxCh0), rangeMin_ch1(minCh1), rangeMax_ch1(maxCh1), maximum(0), maximum2(0), plot(true), sigma(sigma), hysteresis(hysteresis), timeOverThreshold(0), baseline_n(baseline_n), baseline(.0), baseline_rms(.0), risetime(.0), referenceTime(.0), thresholdTime(.0), found(0), rejectedCounter(0), detectorNumber(-1) {};
};

double PreProcess(std::vector<double>& time, std::vector<double>& data, AlgorithmParameters& param) {
  if (time.size()==0) {
    std::cout<<"Size 0"<<std::endl;
    return 0.;
  }
  if (data.size()==0) {
    std::cout<<"Size 0"<<std::endl;
    return 0.;
  }     
  
  ++param.counter;
  double SamplingPeriod = time.at(2) - time.at(1);
  param.threshold_used = (param.detectorNumber==0) ? param.threshold_ch0 : param.threshold_ch1; 
  
  //Polarity
  if (param.threshold_used>0)   param.polarity = 1;
  else  {
    param.polarity = -1;
    param.threshold_used=0-param.threshold_used;
  }
  
  // Compute the baseline
  double baseline=.0;
  double baseline_rms=.0;
  int baseline_nOfP = param.baseline_n * ((double) time.size());
//   baseline_nOfP=100;
  int start_baseline = 1;
  //if (param.counter<2) std::cout<<"N Of Points for baseline: " << baseline_nOfP << " on " << time.size() << std::endl;
  for (uint i= 0; i<baseline_nOfP; ++i) {
    baseline += data.at(start_baseline+i);
    baseline_rms += data.at(start_baseline+i)*data.at(start_baseline+i);
  }
  param.baseline = baseline/baseline_nOfP;
  param.baseline_rms = TMath::Sqrt(baseline_rms/baseline_nOfP);
    
  //Inverting negative signals and removing baseline
  if (param.polarity==-1) {
    for (uint i=0; i<data.size(); ++i) {
      data.at(i) = param.baseline-data.at(i);
    }
  }
  else {
    for (uint i=0; i<data.size(); ++i) {
      data.at(i) = data.at(i)-param.baseline;
    }
  }
  
  std::vector<double>::const_iterator max = std::max_element(data.begin(), data.end());
  double maxTime = time.at(max-data.begin());
  int max_i = max-data.begin();
  param.maximum=*max;
  
  std::vector<double>* gauss_values_pt=NULL;
  if (param.detectorNumber==0) gauss_values_pt=&(param.gauss_values_d0);
  else gauss_values_pt=&(param.gauss_values_d1);
  
  //Adjusting timing intervals if smoothing is required
  if ( param.sigma!=0 && (time.at(2) - time.at(1) != time.at(3) - time.at(2)) ) { //Different sampling periods... needs interpolation to apply digital filters
    double avgSamplingPeriod=.0;
    int avgSamplingPeriodCounter=0;
    for (int i=1; i<time.size(); ++i) {
      //find average sampling period
      avgSamplingPeriod += time.at(i) - time.at(i-1);
      ++avgSamplingPeriodCounter;
    }
    SamplingPeriod = avgSamplingPeriod/avgSamplingPeriodCounter;
    TGraph timeCalibration(time.size());
    for (int i=0; i<time.size(); ++i) {
      timeCalibration.SetPoint(i,time.at(i), data.at(i));
    }
    for (int i=1; i<time.size(); ++i) {
      time.at(i) = time.at(0) + SamplingPeriod*i;
      data.at(i) = timeCalibration.Eval( time.at(i) );
    }   
  }
    
  //Smoothing
  if (gauss_values_pt->size() == 0 && param.sigma!=0) {
    if (param.detectorNumber==1) param.sigma = 0.8e9;   //Fixed for ch0
    TF1 sinc("sinc","[0]*sin(2*pi*[1]*x)/(2*pi*[1]*x)",-50./param.sigma,50./param.sigma);
    sinc.SetParameter(0,1);
    sinc.SetParameter(1,param.sigma);
    int threeSigmaInteger = (10./param.sigma) / SamplingPeriod;
    /* std::cout<< "Detector number: " << param.detectorNumber <<std::endl; */
    /* std::cout<< "Smoothing values: " <<std::endl; */
    /* std::cout<< "Low pass freq: " << param.sigma <<std::endl; */
    /* std::cout<< "Number of points: " << threeSigmaInteger <<std::endl; */
    /* std::cout<< "SamplingPeriod: " << SamplingPeriod <<std::endl; */
    
    gauss_values_pt->resize(2*threeSigmaInteger+1);
    for (int jj=-threeSigmaInteger; jj<=threeSigmaInteger; ++jj) {
      gauss_values_pt->at(threeSigmaInteger+jj) = sinc.Eval(jj * SamplingPeriod);
    }
    gauss_values_pt->at(threeSigmaInteger) = 1;
  }

  std::vector<double> smoothed_data(data.size(),.0); 
  int highLimit = (gauss_values_pt->size()-1)/2;
  if (gauss_values_pt->size() != 0) {
    for (int i=0; i<data.size(); ++i) {
      for (int jj=-highLimit; jj<=highLimit; ++jj) {
        if ((i+jj)>=0 && (i+jj)<data.size()) {
          smoothed_data.at(i) += data.at(i+jj) * gauss_values_pt->at((gauss_values_pt->size()-1)/2+jj);
        }
      }
    }
    std::vector<double>::const_iterator max_smoothed = std::max_element(smoothed_data.begin(), smoothed_data.end());
    param.scaleFactor = param.maximum / (*max_smoothed);
    param.maximum2 = (*max_smoothed)/10;
    for (int i=0; i<data.size(); ++i) {
      data.at(i) = smoothed_data.at(i) * param.scaleFactor;
    } 
  
  }
  
  baseline=.0;
  baseline_rms=.0;
  for (uint i= 0; i<baseline_nOfP; ++i) {
      baseline += data.at(start_baseline+i);
      baseline_rms += data.at(start_baseline+i)*data.at(start_baseline+i);
    }
  param.baseline = baseline/baseline_nOfP;
  param.baseline_rms = TMath::Sqrt(baseline_rms/baseline_nOfP);
  }

  bool IsGoodSignal(std::vector<double>& time, std::vector<double>& data, AlgorithmParameters& param) {
  int i_th,peaks_tmp;
  double thCrossing = comparator(time, data, param.threshold_used, param.hysteresis, param.timeOverThreshold, param.numberOfpeaks, i_th);
  param.thresholdTime = thCrossing;
  
  bool selectedEvent=true;
  if (param.numberOfpeaks!=1) selectedEvent=false;
  if (param.detectorNumber==0) {
    if (param.maximum<param.rangeMin_ch0 || param.maximum>param.rangeMax_ch0) selectedEvent=false;
  }
  else if (param.detectorNumber==1) {
    if (param.maximum<param.rangeMin_ch1 || param.maximum>param.rangeMax_ch1) selectedEvent=false;
    if (param.timeOverThreshold<0.2e-9) selectedEvent==false;
  }
  else selectedEvent=false;
  
  if (selectedEvent == false) {
    param.timeOverThreshold=-1;
    param.thresholdTime=-1;
    param.risetime=-1;
  }  
  return selectedEvent;
}

double ComputeExactTimeCFD(std::vector<double>& time, std::vector<double>& data, AlgorithmParameters& param) {
  //Smoothing and preprocessing
  PreProcess(time, data, param);
  if (IsGoodSignal(time, data, param) == false) return -1;
 
 std::vector<double> smoothed_data_norm(data.size(),.0); 
 std::vector<double>::const_iterator max_smoothed = std::max_element(data.begin(), data.end());
 param.scaleFactor = 1. / (*max_smoothed);
 for (int i=0; i<data.size(); ++i) {
   smoothed_data_norm.at(i) = data.at(i) * param.scaleFactor;
 } 
 
 /**
 * Compute rise time
 **/
 int numberOfpeaks10, numberOfpeaks90, numberOfpeaksTmp, i_th_10, i_th_90;
 double timeOverThreshold10, timeOverThreshold90;
 double time10 = comparator(time, smoothed_data_norm, 0.1, 0.01/param.scaleFactor, timeOverThreshold10, numberOfpeaks10, i_th_10);
 double time90 = comparator(time, smoothed_data_norm, 0.9, 0.01/param.scaleFactor, timeOverThreshold90, numberOfpeaks90, i_th_90);
 if (numberOfpeaks10==1 && numberOfpeaks90==1 && (timeOverThreshold10-timeOverThreshold90)>0 && (timeOverThreshold10-timeOverThreshold90)<20e-9) {
   param.risetime = time90 - time10;
 }
 else
   param.risetime = -1;
 
 /**  
 * CFD
 **/
  double t_cfd=0;
  double t_simple_tmp=0;
  int cfdCounter=0;
  int i_th=0;
  if (param.detectorNumber==0) {
    for (double cfdRatioTmp = 0.45; cfdRatioTmp<=0.45; cfdRatioTmp+=0.1) {
      t_simple_tmp = comparator(time, smoothed_data_norm, cfdRatioTmp, param.hysteresis*param.scaleFactor, timeOverThreshold10, numberOfpeaksTmp, i_th);
      if (numberOfpeaksTmp!=1) {
        continue;
      }
      t_cfd += t_simple_tmp;
      ++cfdCounter;
    }
    t_cfd = t_cfd / cfdCounter;
    
    return t_cfd;
  }
  else {
    for (double cfdRatioTmp = param.cfdRatio; cfdRatioTmp<=param.cfdRatio; cfdRatioTmp+=0.1) {
      t_simple_tmp = comparator(time, smoothed_data_norm, cfdRatioTmp, param.hysteresis*param.scaleFactor, timeOverThreshold10, numberOfpeaksTmp, i_th);
      if (numberOfpeaksTmp!=1) {
        continue;
      }
      t_cfd += t_simple_tmp;
      ++cfdCounter;
    }
    t_cfd = t_cfd / cfdCounter;
    
    return t_cfd;
  }
}

