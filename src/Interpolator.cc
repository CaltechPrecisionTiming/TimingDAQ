#include "Interpolator.hh"
#include <TMath.h>

void Interpolator::init(unsigned _nSamples,double _tMin, double _tMax, float* _samples){
  nSamples = _nSamples; // total number of samples
  tMin = _tMin; // time of first sample
  tMax = _tMax; // time of last sample
  samples = _samples;	// pointer to the array with sampled values

  deltaT = (tMax - tMin)/(double)(nSamples - 1); // sampling time interval
};

// return interpolated value for a generic time t within [tMin, tMax]

double Interpolator::f(double t){
  if(t < tMin) t = tMin;
  if(t > tMax) t = tMax;
  double s = (t - tMin)/deltaT;

  int iSampleMin = (int)s - sampleRange;//interpolation window lower edge
  if(iSampleMin < 0) iSampleMin = 0;
  int iSampleMax = (int)s + sampleRange;//interpolation window higher edge
  if(iSampleMax > (int)nSamples) iSampleMax = nSamples;

  double result = 0.;
  for(unsigned iSample = iSampleMin; (int)iSample != iSampleMax; ++ iSample){
    double sNorm = (s - (double)iSample)*TMath::Pi()*cutFreqRel;
    if(sNorm == 0.) result += samples[iSample];
    //else result += samples[iSample]*sin(sNorm)/sNorm;
    else result += samples[iSample]*Sinc(sNorm);
  }
  return result;
};

double Interpolator::Sinc(double x){

	const double a2 = -1./6.;
	const double a4 = +1./120.;
	const double a6 = -1./5040.;

	if(abs(x) > 0.001) return sin(x)/x;
	double x2 = x*x;
	return ((a6*x2 + a4)*x2 + a2)*x2 +1; // Taylor expansion of sinc(x) function
};
