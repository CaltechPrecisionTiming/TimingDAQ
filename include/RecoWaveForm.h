#ifndef RecoWaveForm_h
#define RecoWaveForm_h

#include "TGraph.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include <Riostream.h>
#include <vector>
#include <algorithm>
#include <iomanip>



class ChannelDRS {
  
 public:
  ChannelDRS(int, double*, int, double*, double, double, double, double, int);
  ~ChannelDRS();
  double parTvsR(int i) { if(i>=0 && i<10) return parTvsR_[i]; else return 0.; };
  double parAvsR(int i) { if(i>=0 && i<10) return parAvsR_[i]; else return 0.; };
  double ratioMin()     { return ratioMin_; };
  double ratioMax()     { return ratioMax_; };
  double ampThreshold() { return ampThreshold_; };
  double ratioT0()      { return ratioT0_; };
  double timeStep()     { return timeStep_; };
  int    ratioStep()    { return ratioStep_; };
  void   SetParTvsR(int i, double x) { if(i>=0 && i<10) parTvsR_[i] = x; };
  void   SetParAvsR(int i, double x) { if(i>=0 && i<10) parAvsR_[i] = x; };
 private:
  double parTvsR_[10];
  double parAvsR_[10];
  double ratioMin_;
  double ratioMax_;
  double ampThreshold_;
  double ratioT0_;
  double timeStep_;
  int    ratioStep_;
};



ChannelDRS::ChannelDRS(int np1, double *vp1, int np2, double *vp2, double rmin, double rmax, double thr,
		 double tstep, int rstep)
{
  for(int i=0; i<10; i++){
    parTvsR_[i] = 0;
    parAvsR_[i] = 0;
  }
  for(int i=0; i<np1; i++){
    if(i<10) parTvsR_[i] = vp1[i];
  }
  for(int i=0; i<np2; i++){
    if(i<10) parAvsR_[i] = vp2[i];
  }
  ratioMin_  = rmin;
  ratioMax_  = rmax;
  timeStep_  = tstep;
  ratioStep_ = rstep;

  // fraction of amplitude that will correspond to pulse timing
  ampThreshold_ = thr;
  if(thr<1e-3 && thr>0.8){
    // protect against meaningless values
    ampThreshold_ = 0.1;
  }

  TF1 *fTmpA = new TF1("fTmpA","pol9", 1e-3, 0.8);
  for(int i=0; i<10; i++){
    fTmpA->SetParameter(i, parAvsR_[i]);
  }
  ratioT0_ = fTmpA->GetX(ampThreshold_);

  TF1 *fTmpT = new TF1("fTmpT","pol9", 1e-3, 0.8);
  fTmpT->SetParameter(0, parTvsR_[0]);
  for(int i=1; i<10; i++){
    fTmpT->SetParameter(i, parTvsR_[i]);
  }

  delete fTmpA;
  delete fTmpT;
}



ChannelDRS::~ChannelDRS()
{
}



class RecoWaveForm {
  
 public :
  RecoWaveForm(double*, double*, ChannelDRS* );
  ~RecoWaveForm();
  double tStart()          { return tStart_;    };
  double amp( int i)       { return amp_[i] ;   };
  int    imin()            { return imin_;      };
  double pedMean()         { return pedMean_;   };
  double pedRMS()          { return pedRMS_;    };
  double aReco()           { return aReco_;     };
  double tReco()           { return tReco_;     };
  int    nRatios()         { return rv_.size(); };
  double ratioValue(int i) { return rv_[i];     };
  double ratioError(int i) { return re_[i];     };
  double timeValue(int i)  { return tv_[i];     };
  double sampleValue(int i){ return av_[i];     };
  
 private:
  double amp_[1024];
  double tStart_;
  int    imin_;
  double pedMean_;
  double pedRMS_;
  double tReco_;
  double aReco_;
  std::vector<double> rv_;
  std::vector<double> re_;
  std::vector<double> tv_;
  std::vector<double> av_;
};



RecoWaveForm::RecoWaveForm(double *wf, double *t, ChannelDRS *ch)
{
  TGraph *grTmp = new TGraph(1024, t, wf);
  tStart_ = t[0];
  for(int i=0; i<1024; i++){
    double x = tStart_ + i * ch->timeStep();
    if(x<=t[1023]){
      amp_[i] = grTmp->Eval(x);
    }else{
      amp_[i] = 0;
    }
  }
  delete grTmp;

  double ymin = 1e+9;
  imin_ = 0;
  for(int i=0; i<1024; i++){
    if(amp_[i]<ymin){
      ymin = amp_[i];
      imin_ = i;
    }
  }

  // RMS of non-zero samples in original waveform

  double sum0 = 0;
  double sum1 = 0;
  double sum2 = 0;
  double tMin = tStart_ + std::max(0,imin_-200) * ch->timeStep();
  double tMax = tStart_ + std::max(0,imin_-150) * ch->timeStep();
  for(int i=0; i<1024; i++){
    if(fabs(wf[i])>1e-9 && t[i]>tMin && t[i]<tMax){
      sum0 += 1.;
      sum1 += wf[i];
      sum2 += wf[i] * wf[i];
    }
  }
  pedMean_ = 0;
  pedRMS_  = 1e+9;
  if(sum0>0){
    pedMean_ = sum1 / sum0;
    pedRMS_  = sqrt( sum2 / sum0 - pedMean_ * pedMean_ );
  }

  for(int i=0; i<1024; i++){
    amp_[i] -= pedMean_;
  }

  
  std::vector<int> indx;
  for(int ix=0; ix<1024; ix++){
    bool isBad = true;
    if(ix < std::min( imin_,1024 - ch->ratioStep() )){
      if(amp_[ix + ch->ratioStep()] < -3. * pedRMS_ && amp_[ix] < -3 * pedRMS_
	 && amp_[ix + ch->ratioStep()]>0.8*amp_[imin_]){
	double ratio = amp_[ix] / amp_[ix + ch->ratioStep()];
	if(ratio > 0.05 && ratio < 0.8 ){
	  isBad = false;
	}
      }
    }
    if(isBad) indx.push_back(ix);
  }
  std::sort(indx.begin(),indx.end());
	
  int idstart = 0;
  int igap = 0;
  for(unsigned int ix=0; ix<indx.size()-1; ix++){
    int tmp = indx[ix+1]-indx[ix];
    if(tmp>=igap){
      igap = tmp;
      idstart = indx[ix];
    }
  }

  sum0 = 0;
  sum1 = 0;
  sum2 = 0;

  rv_.clear();
  re_.clear();
  tv_.clear();
  av_.clear();
  
  for(int ix=idstart+1; ix<std::min(idstart+igap,1024-ch->ratioStep()); ix++){
    double tmp   = tStart_ + ix * ch->timeStep();
    double ratio = amp_[ix] / amp_[ix+ch->ratioStep()];
    double err1  = sqrt(ratio * ratio * (pow(pedRMS_/amp_[ix],2) + pow(pedRMS_/amp_[ix+ch->ratioStep()],2)));
    double err2  = pedRMS_*(amp_[ix]-amp_[ix+ch->ratioStep()])/(pow(amp_[ix+ch->ratioStep()],2));
    double err   = sqrt( err1 * err1 + err2 * err2 );
    sum0 += 1./(err * err);
    sum1 += tmp / (err * err);
    rv_.push_back( ratio );
    re_.push_back( err );
    tv_.push_back( tmp );
    av_.push_back( amp_[ix] );
  }

  TGraphErrors *grTmpT = new TGraphErrors();
  TGraphErrors *grTmpA = new TGraphErrors();
  int nGoodPoints = 0;
  for(unsigned int iv=0; iv<rv_.size(); iv++){
    grTmpT->SetPoint(iv, rv_[iv], tv_[iv]);
    grTmpT->SetPointError(iv, re_[iv], 0.025);
    grTmpA->SetPoint(iv, rv_[iv], av_[iv]);
    grTmpA->SetPointError(iv, re_[iv], pedRMS_);
    if( rv_[iv]>ch->ratioMin() && rv_[iv]<ch->ratioMax() ){
      nGoodPoints++;
    }
  }
  if(nGoodPoints>0){
    TF1 *fTmpA = new TF1("fTmpA","[0]*([1]*pow(x,1)+[2]*pow(x,2)+[3]*pow(x,3)+[4]*pow(x,4)+[5]*pow(x,5)+[6]*pow(x,6)+[7]*pow(x,7)+[8]*pow(x,8)+[9]*pow(x,9))", 1e-3, 0.8);
    
    for(int i=0; i<10; i++){
      fTmpA->SetParameter(i, ch->parAvsR(i));
    }
    for(int i=1; i<10; i++){
      fTmpA->FixParameter(i, ch->parAvsR(i));
    }
    grTmpA->Fit("fTmpA","EQ","",ch->ratioMin(),ch->ratioMax());
    aReco_ = fTmpA->GetParameter(0);

    TF1 *fTmpT = new TF1("fTmpT","pol9", 1e-3, 0.8);
    for(int i=0; i<10; i++){
      fTmpT->SetParameter(i, ch->parTvsR(i));
    }
    for(int i=1; i<10; i++){
      fTmpT->FixParameter(i, ch->parTvsR(i));
    }
    grTmpT->Fit("fTmpT","EQ","",ch->ratioMin(),ch->ratioMax());
    tReco_ = fTmpT->Eval(ch->ratioT0());
    delete fTmpA;
    delete fTmpT;
  }else{
    aReco_ = 0;
    tReco_ = 1e+9;
  }
 
  delete grTmpT;   
  delete grTmpA;   
}


RecoWaveForm::~RecoWaveForm()
{
}

#endif 
