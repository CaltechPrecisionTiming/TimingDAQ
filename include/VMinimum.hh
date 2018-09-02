
double VMinimum(double tMin, double tMax,
	unsigned nSamples, Float_t * time, Float_t * channel, bool polarity = false )
{
	Interpolator voltage;
	voltage.init(nSamples, time[0], time[nSamples-1], channel);

	double tStep = (time[nSamples-1] - time[0])/(double)(nSamples-1)/10.;

	double vMin = 0.;

	if ( polarity )
	{
		for(double t = tMin; t <= tMax; t += tStep)
		{
			if(voltage.f(t) > vMin) vMin = voltage.f(t);
		}
	}
	else
	{//default
		for(double t = tMin; t <= tMax; t += tStep)
		{
			if(voltage.f(t) < vMin) vMin = voltage.f(t);
		}
	}
	return vMin;
}
