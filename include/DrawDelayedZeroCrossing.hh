
void DrawDelayedZeroCrossing(double delay,double gain,double tMin, double tMax,unsigned nSamples,Float_t * time, Float_t * voltage
		,unsigned nSamplesInt,Float_t * timeInt, Float_t * voltageInt
		, std::string pictName = "DZC"){

	TCanvas * IntScopeCanvas = new TCanvas("DZCCanvas","DZCCanvas",50,50,500,500);

 	TH1D intScope("DZC","DZC",100,tMin,tMax);

 	intScope.SetMaximum(+40.);
 	intScope.SetMinimum(-200.);
 	intScope.SetStats(false);

    IntScopeCanvas->cd();

    Float_t delTime[nSamples];
    Float_t delVoltage[nSamples];
    Float_t delTimeInt[nSamplesInt];
    Float_t delVoltageInt[nSamplesInt];

    for(unsigned iS = 0; iS != nSamples; ++ iS) {
    	delTime[iS] = time[iS] + delay;
    	delVoltage[iS] = voltage[iS]/gain;
    }

    for(unsigned iS = 0; iS != nSamplesInt; ++ iS) {
    	delTimeInt[iS] = timeInt[iS] + delay;
    	delVoltageInt[iS] = voltageInt[iS]/gain;
    }


    TGraph * scopeTrace = new TGraph(nSamples, time, delVoltage);
    TGraph * intScopeTrace = new TGraph(nSamplesInt, timeInt, delVoltageInt);

    TGraph * delScopeTrace = new TGraph(nSamples, delTime, voltage);
    TGraph * delIntScopeTrace = new TGraph(nSamplesInt, delTimeInt, voltageInt);


	scopeTrace->SetMarkerStyle(20);
	scopeTrace->SetMarkerSize(0.5);
	scopeTrace->SetMarkerColor(4);

	intScopeTrace->SetMarkerStyle(20);
	intScopeTrace->SetMarkerSize(0.2);
	intScopeTrace->SetMarkerColor(2);

	delScopeTrace->SetMarkerStyle(20);
	delScopeTrace->SetMarkerSize(0.5);
	delScopeTrace->SetMarkerColor(4);

	delIntScopeTrace->SetMarkerStyle(20);
	delIntScopeTrace->SetMarkerSize(0.2);
	delIntScopeTrace->SetMarkerColor(2);


    intScope.Draw();
    intScopeTrace->Draw("PSAME");
    scopeTrace->Draw("PSAME");
    delIntScopeTrace->Draw("PSAME");
    delScopeTrace->Draw("PSAME");



    std::string fullFileName = pictName + ".pdf";
    IntScopeCanvas->SaveAs(fullFileName.c_str());

    delete IntScopeCanvas;


    return;

}; // end DrawScope
