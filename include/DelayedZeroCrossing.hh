int DelayedZeroCrossing (double thresh, double delay, double gain, double tMin, double tMax,
	unsigned nSamples, Float_t * time, Float_t * channel, double &timeZC)
{

	Interpolator voltage;
	voltage.init(nSamples, time[0], time[nSamples-1], channel);
	
	double tStep = (time[nSamples-1] - time[0])/(double)(nSamples-1)/4.;
	double tStepInit = tStep;
	double t = tMin;
	unsigned nIterations = 0;
	for(;;){
		//cout << "tStep: " << tStep << endl;
		for(; (voltage.f(t) - thresh)*tStep > 0.; t += tStep){	
			//cout << "t: " << t << endl;
			if(t<tMin) return -1;
			if(t>tMax) return -2;	
		}
		if(abs(tStep) < 0.004) break;
		tStep = - tStep/2.;
		++nIterations;
		if(nIterations > 1000) return -3;
	}
		
	tStep = tStepInit;
	t += tStep;
	
	nIterations = 0;
	for(;;){
		//cout << "tStep: " << tStep << endl;
		for(; (voltage.f(t) - gain*voltage.f(t-delay))*tStep < 0.; t += tStep){	
			//cout << "t: " << t << endl;
			if(t<tMin) return -4;
			if(t>tMax) return -5;	
		}
		if(abs(tStep) < 0.004) break;
		tStep = - tStep/2.;
		++nIterations;
		if(nIterations > 1000) return -6;
	}
	
	timeZC = t;
	
	return 0;
	
	
}