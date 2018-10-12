int TimeOverThreshold(double tThresh, double tMin, double tMax,
	unsigned nSamples, Float_t * time, Float_t * channel, double &time1, double &time2)

{
	Interpolator voltage;
	voltage.init(nSamples, time[0], time[nSamples-1], channel);

	double tStep = (time[nSamples-1] - time[0])/(double)(nSamples-1)/4.;
	double tStepInit = tStep;
	double t = tMin;
	unsigned nIterations = 0;

	while( nIterations <= 1000)
	{
		std::cout << "tStep: " << tStep << std::endl;
		std::cout << "t: " << t << " " << voltage.f(t) << " " << channel[10] << " " <<  tThresh << std::endl;
		while((voltage.f(t) - tThresh)*tStep < 0.)
		{
			std::cout << nIterations << " t: " << t  << " " << voltage.f(t) << " " << tThresh << std::endl;
			if(t<tMin) return -1;
			if(t>tMax) return -2;
			t += tStep;
		}
		if( fabs(tStep) < 0.0004) break;
		tStep =- tStep/2.;
		nIterations++;
	}

	if(nIterations == 1000) return -3;//iterations reached maximum
	time1 = t;


	std::cout << "===================================" << std::endl;
	std::cout << "time1: " << time1 << " f(t) = " << voltage.f(time1) << std::endl;
	std::cout << "===================================" << std::endl;

	tStep = tStepInit;
	t += tStep;

	nIterations = 0;
	while( nIterations <= 1000)
	{
		//cout << "tStep: " << tStep << endl;
		while( (voltage.f(t) - tThresh)*tStep > 0 )
		{
			//cout << "t: " << t << endl;
			if(t<tMin) return -4;
			if(t>tMax) return -5;
			t += tStep;
		}
		if( fabs(tStep) < 0.0004 ) break;
		tStep =- tStep/2.;
		nIterations++;
	}

	if(nIterations == 1000) return -6;//iterations reached maximum
	time2 = t;
/*
	std::cout << "===================================" << std::endl;
	std::cout << "time2: " << time2 << " f(t) = " << voltage.f(time2) << std::endl;
	std::cout << "===================================" << std::endl;
*/
	return 0;

}
