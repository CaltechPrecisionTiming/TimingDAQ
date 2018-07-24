/*
The Interpolator class can be used to interpolate a sampled signal using the Sampling Theorem.
The samples are assumed to be taken at constant frequency
(For an explanation of the Sampling Theorem see for example
National Semiconductor Application Note 236, January 1980)
This is From Luciano Ristori
*/


//
// my implementation of sinc(x) = sin(x)/x
//

#ifndef INTERPOLATOR_HH
#define INTERPOLATOR_HH

#include <stdlib.h>

class Interpolator {
	public:
		unsigned nSamples; // total number of samples
		float* samples; // pointer to the array with sampled values
		double tMin, tMax; // times of first and last samples

		double deltaT; // sampling time interval
		const int sampleRange = 16; // range of samples to use for interpolation
		double cutFreqRel = 1.0; // cut frequency relative to max frequency (half of sampling freq)

		Interpolator(){}; // default constructor
		// initialize the interpolator with sampled data
		double Sinc(double x);
		void init(unsigned _nSamples, double _tMin, double _tMax, float* _samples);
		// return interpolated value for a generic time t within [tMin, tMax]
		double f(double t);
};

#endif
