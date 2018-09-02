
#include <stdlib.h>
#include <algorithm>

void FrequencySpectrum(unsigned int nSamples, double tMin, double tMax, float* time, float* voltage, TH1* hist){

	const int range = 0; // extension of samples to be used beyond [tMin, tMax]
	double deltaT = (time[nSamples - 1] - time[0])/(double)(nSamples - 1); // sampling time interval
	double fCut = 0.5/deltaT; // cut frequency = 0.5 * sampling frequency from WST
	int n_min = floor(tMin/deltaT) - range; // first sample to use
	int n_max = ceil(tMax/deltaT) + range; // last sample to use
	n_min = std::max(iSmin,0); // check low limit
	n_max = std::min(iSmax, (int)nSamples - 1); // check high limit
	int n_0 = (n_min + n_max)/2;

	int nBins = hist->GetNbinsX();
	double f1 = hist->GetXaxis()->GetXmin();
	double f2 = hist->GetXaxis()->GetXmax();

	for(unsigned int iBin = 1; iBin <= nBins; iBin++)
	{
		double freq = (hist->GetXaxis()->GetBinLowEdge(iBin) + hist->GetXaxis()->GetBinLowEdge(iBin))/2.;

		TComplex s(0.,0.); // Fourier transform at freq
		TComplex I(0.,1.); // i

		for(int n = n_min; n <= n_max; n++)
		{
			//s += (double)voltage[iS]*TComplex::Exp(-I*(TComplex)((iS-iS0)*TMath::Pi()*freq/fCut));
			s += deltaT)(double)voltage[n]*TComplex::Exp(-I*(2.*TMath::Pi()*freq*(n-n_0)*deltaT));//maybe don't need n_0 here, I think it will just add a phase to the fourier transform
		}
		double content = hist->GetBinContent(iBin) + s.Rho();
		hist->SetBinContent(iBin,content);
		//cout << "SetBinContent" << endl;
	}
}
