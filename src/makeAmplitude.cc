#ifdef __MAKECINT__
#pragma link C++ class vector<vector<float> >+;
#endif

'''HAVE to be deleted at the end'''

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TMath.h"
#include "TF1.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TVirtualFitter.h"
#include "TPaveText.h"
#include "TProfile.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TColor.h"
#include "TGraphErrors.h"
#include "TLatex.h"
#include "TGaxis.h"
#include "TPad.h"
#include <math.h>

int FindRisingEdge( int n, int binMax, TH1F *a);

void MakeAmplitudePlot(std::string filename, std::string plotname) {
  // Get the tree


  TFile *inputfile = new TFile(filename.c_str(),"READ");
  TTree *tree = (TTree*)inputfile->Get("pulse");

  // get the variables from the ntuple
  Float_t amp[36];

  tree->SetBranchAddress("amp",&amp);

  //create histograms

  TH2F *AmplitudeMap = new TH2F("AmplitudeMap","; X; Y", 8, 0, 8, 8, 0, 8);

  std::vector<TH1F*> Amplitudes;
  for (int i=0; i < 36; ++i) {
    Amplitudes.push_back( new TH1F( Form("Amplitudes_%d",i), ";Amplitude;Number of Events", 100, 0, 2500));

    Amplitudes[i]->Sumw2();
    }

  //read all entries and fill the histograms
  Long64_t nentries = tree->GetEntries();
  std::cout<<"Number of events in Physics Sample: "<<nentries<<std::endl;

  //  for (Long64_t iEntry=0;iEntry<nentries;iEntry++) {
  for (Long64_t iEntry=442;iEntry<443;iEntry++) {
      tree->GetEntry(iEntry);

      for(int pixel = 0; pixel<36; pixel++)
	Amplitudes[pixel]->Fill(amp[pixel]);
  }


  std::cout<<"AAAAAAAAAAA "<< FindRisingEdge(1024, 30, Amplitudes[3] )<<std::endl;


  for(int i = 0; i<6; i++)
    for(int j = 0; j<6; j++)
      {
	if(i == 3 && j == 0) AmplitudeMap->SetBinContent(5,1, Amplitudes[1]->GetMean());
	if(i == 4 && j == 0) AmplitudeMap->SetBinContent(4,1, Amplitudes[2]->GetMean());
	if(i == 5 && j == 0) AmplitudeMap->SetBinContent(3,1, Amplitudes[3]->GetMean());

	if(i == 5 && j == 1) AmplitudeMap->SetBinContent(3,2, Amplitudes[4]->GetMean());
	if(i == 4 && j == 1) AmplitudeMap->SetBinContent(4,2, Amplitudes[5]->GetMean());
	if(i == 3 && j == 1) AmplitudeMap->SetBinContent(5,2, Amplitudes[6]->GetMean());

	if(i == 3 && j == 2) AmplitudeMap->SetBinContent(5,3, Amplitudes[7]->GetMean());
	if(i == 4 && j == 2) AmplitudeMap->SetBinContent(4,3, Amplitudes[10]->GetMean());
	if(i == 5 && j == 2) AmplitudeMap->SetBinContent(3,3, Amplitudes[11]->GetMean());

	if(i == 5 && j == 3) AmplitudeMap->SetBinContent(3,4, Amplitudes[12]->GetMean());
	if(i == 4 && j == 3) AmplitudeMap->SetBinContent(4,4, Amplitudes[13]->GetMean());
	if(i == 3 && j == 3) AmplitudeMap->SetBinContent(5,4, Amplitudes[14]->GetMean());

	if(i == 3 && j == 4) AmplitudeMap->SetBinContent(5,5, Amplitudes[15]->GetMean());
	if(i == 4 && j == 4) AmplitudeMap->SetBinContent(4,5, Amplitudes[16]->GetMean());
	if(i == 5 && j == 4) AmplitudeMap->SetBinContent(3,5, Amplitudes[19]->GetMean());

	if(i == 5 && j == 5) AmplitudeMap->SetBinContent(3,6, Amplitudes[20]->GetMean());
	if(i == 4 && j == 5) AmplitudeMap->SetBinContent(4,6, Amplitudes[21]->GetMean());
	if(i == 3 && j == 5) AmplitudeMap->SetBinContent(5,6, Amplitudes[22]->GetMean());

	if(i == 2 && j == 5) AmplitudeMap->SetBinContent(6,6, Amplitudes[23]->GetMean());
	if(i == 2 && j == 4) AmplitudeMap->SetBinContent(6,5, Amplitudes[24]->GetMean());
	if(i == 2 && j == 3) AmplitudeMap->SetBinContent(6,4, Amplitudes[25]->GetMean());
	if(i == 2 && j == 2) AmplitudeMap->SetBinContent(6,3, Amplitudes[28]->GetMean());
	if(i == 2 && j == 1) AmplitudeMap->SetBinContent(6,2, Amplitudes[29]->GetMean());
	if(i == 2 && j == 0) AmplitudeMap->SetBinContent(6,1, Amplitudes[30]->GetMean());

	if(i == 1 && j == 0) AmplitudeMap->SetBinContent(7,1, Amplitudes[31]->GetMean());
	if(i == 1 && j == 1) AmplitudeMap->SetBinContent(7,2, Amplitudes[32]->GetMean());
	if(i == 1 && j == 2) AmplitudeMap->SetBinContent(7,3, Amplitudes[33]->GetMean());
      }

  AmplitudeMap->Draw();

  Amplitudes[0]->SaveAs("test.root");
  AmplitudeMap->SaveAs("test2.root");

}



int main(int argc, char **argv)
{
  MakeAmplitudePlot(argv[1], argv[2]);
}

int FindRisingEdge( int n, int binMax, TH1F *a) {


  if (n <= 0 || !a) return -1;
  float xmin = a->GetBinContent(0);
  int loc = -99;
  for (int i = binMax-10; i <binMax; i++)
  { // sometimes there is noise, check that it is rising in three bins
    if ( a->GetBinContent(i) > 5. && a->GetBinContent(i+1) > a->GetBinContent(i) && a->GetBinContent(i+2) < a->GetBinContent(i+1) )
      {
	loc = i;
	break;
      }
  }
  return loc;
}
