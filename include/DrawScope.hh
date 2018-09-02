
void DrawScope(unsigned nSamples,double tMin, double tMax, Float_t * time, Float_t * voltage, std::string pictName = "scope"){

   TCanvas * scopeCanvas = new TCanvas("scopeCanvas","scopeCanvas",50,50,500,500);
   TH1D scope("scope","scope",100,tMin,tMax);

 	//scope.SetMaximum(+40.);
 	//scope.SetMinimum(-200.);
 	//scope.SetStats(false);

    TGraph * scopeTrace = new TGraph(nSamples, time, voltage);
    //scopeCanvas->cd();

    //scope.Draw();
    scopeTrace->Draw("AC*");
    std::string fullFileName = pictName + ".pdf";
    scopeCanvas->SaveAs(fullFileName.c_str());

    delete scopeCanvas;

    return;

}; // end DrawScope
