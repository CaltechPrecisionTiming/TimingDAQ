
void DrawInterpolation(double tMin, double tMax,unsigned nSamples,Float_t * time, Float_t * voltage
		,unsigned nSamplesInt,Float_t * timeInt, Float_t * voltageInt
		, std::string pictName = "IntScope"){

	TCanvas * IntScopeCanvas = new TCanvas("IntScopeCanvas","IntScopeCanvas",50,50,500,500);

 	TH1D intScope("IntScope","IntScope",100,tMin,tMax);

 	//intScope.SetMaximum(+40.);
 	//intScope.SetMinimum(-200.);
 	//intScope.SetStats(false);

  //IntScopeCanvas->cd();

  TGraph * scopeTrace = new TGraph(nSamples, time, voltage);
  TGraph * intScopeTrace = new TGraph(nSamplesInt, timeInt, voltageInt);

	scopeTrace->SetMarkerStyle(20);
	scopeTrace->SetMarkerSize(0.5);
	scopeTrace->SetMarkerColor(kBlue);

	intScopeTrace->SetMarkerStyle(20);
	intScopeTrace->SetMarkerSize(0.2);
	intScopeTrace->SetMarkerColor(kRed);

  //intScope.Draw();
  intScopeTrace->Draw("AP");
  scopeTrace->Draw("PSAME");

  std::string fullFileName = pictName + ".pdf";
  IntScopeCanvas->SaveAs(fullFileName.c_str());

  delete IntScopeCanvas;
	return;

}; // end DrawScope
