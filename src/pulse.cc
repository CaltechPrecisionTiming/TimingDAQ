#define pulse_cxx
#include "pulse.hh"
#include "RecoWaveForm.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TProfile.h>
#include <TGraphErrors.h>
#include <TGraph.h>
#include <TTree.h>


using namespace std;

void pulse::Loop()
{
//   In a ROOT session, you can do:
//      root> .L pulse.C
//      root> pulse t
//      root> t.GetEntry(12); // Fill t data members with entry number 12
//      root> t.Show();       // Show values of entry 12
//      root> t.Show(16);     // Read and show values of entry 16
//      root> t.Loop();       // Loop on all entries
//

//     This is the loop skeleton where:
//    jentry is the global entry number in the chain
//    ientry is the entry number in the current Tree
//  Note that the argument to GetEntry must be:
//    jentry for TChain::GetEntry
//    ientry for TTree::GetEntry and TBranch::GetEntry
//
//       To read only selected branches, Insert statements like:
// METHOD1:
//    fChain->SetBranchStatus("*",0);  // disable all branches
//    fChain->SetBranchStatus("branchname",1);  // activate branchname
// METHOD2: replace line
//    fChain->GetEntry(jentry);       //read all branches
//by  b_branchname->GetEntry(ientry); //read only this branch
   if (fChain == 0) return;

   Long64_t nentries = fChain->GetEntriesFast();

   Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
      // if (Cut(ientry) < 0) continue;
   }
}


TTree* pulse::GetOutputTree()
{
  return tOut;
}

void pulse::SimpleCheck(int channelID = 10, std::string channelName = "ch10")
{
  if (fChain == 0) return;
  //Long64_t nentries = fChain->GetEntries();
  Long64_t nentries = 2000;
  Long64_t nbytes = 0, nb = 0;


  TH1D *hist001 = new TH1D( ("h_recoAmplitude_" + channelName).c_str(), " reco amplitude " , 1000, 0., 10000.);
  TH1D *hist002 = new TH1D( ("h_recoTime_" + channelName).c_str() , " reco time " , 1000, 0., 200.);
  TH1D *hist003 = new TH1D( ("h_maxSample_" + channelName).c_str() , " maximum sample " , 1000, 0., 3000.);
  TH1D *hist004 = new TH1D( ("h_pedestalMean_" + channelName).c_str() , " pedestal mean " , 1000, -100., 100.);
  TH1D *hist005 = new TH1D( ("h_pedestalRMS_" + channelName).c_str() , " pedestal rms " , 1000, 0., 100.);
  TH1D *hist006 = new TH1D( ("h_nRatios_" + channelName).c_str() , " n Ratios " , 30, -0.5, 29.5);
  
  TH1D *hist010 = new TH1D( ("h_nTracks_" + channelName).c_str() , " ntracks " , 10, -0.5, 9.5);
  TH1D *hist011 = new TH1D( ("h_log10Chi2_" + channelName).c_str(), " log10(chi2) " , 1000, -7, 4);
  TH2D *hist012 = new TH2D( ("h_trackYvsX_" + channelName).c_str() , " track Y vs X " , 1000, -10000, 40000, 1000, -10000, 40000);
  TH2D *hist013 = new TH2D( ("h_trackYvsX_withLYSO_" + channelName).c_str() , " track Y vs X with LYSO hit" , 1000, -10000, 40000, 1000, -10000, 40000);
  
  tOut = new TTree( (channelName + "_Tree").c_str(), (channelName + "_Tree").c_str() );
  double b_event     = 0;
  double b_recoAmp   = 0;
  double b_recoTime  = 0;
  double b_maxSample = 0;
  double b_pedMean   = 0;
  double b_pedRMS    = 0;
  double b_nRatios   = 0;
  double b_nTracks   = 0;
  double b_log10chi2 = 0;

  tOut->Branch("event", &b_event);
  tOut->Branch( ("recoAmp_" + channelName).c_str(), &b_recoAmp);
  tOut->Branch( ("recoTime_" + channelName).c_str(), &b_recoTime);
  tOut->Branch( ("maxSample_" + channelName).c_str(), &b_maxSample);
  tOut->Branch( ("pedestalMean_" + channelName).c_str(), &b_pedMean);
  tOut->Branch( ("pedestalRMS_" + channelName).c_str(), &b_pedRMS);
  tOut->Branch( ("nRatios_" + channelName).c_str(), &b_nRatios);
  tOut->Branch( ("nTracks_" + channelName).c_str(), &b_nTracks);
  tOut->Branch( ("log10Chi2_" + channelName).c_str(), &b_log10chi2);
  
  // Setup DRS channel for reconstruction with Ratios. It is assumed
  // to be a SiPM attached to LYSO (not MCP)

  int    dN = 3;
  double dT = 0.2;
  double parT[5] = { -1.20339, 3.94932, -4.77487, 5.69944, -0.630818 };
  double parA[5] = { 0, 0.0330576, 0.292408, 0.651366, 0.000751415 };
  ChannelDRS *ch = new ChannelDRS(5, parT, 5, parA, 0.1, 0.4, 0.1, dT, dN );

  
  for (Long64_t jentry=0; jentry<nentries;jentry++) {
    
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    if (jentry % 100 == 0) cout << "Processing Event " << jentry << " of " << nentries << "\n";
    nb = fChain->GetEntry(jentry);   nbytes += nb;

    double waveForm[1024];
    double timeAxis[1024];
    int group = channelID / 8;
    for(int is=0; is<1024; is++){
      waveForm[is] = channel[channelID][is];
      timeAxis[is] = time[group][is];
      if (jentry==0)
	std::cout<<"entry 0, channelID=" << channelID << ", wave[" << is << "] = " << channel[channelID][is] << std::endl;
    }
    
    RecoWaveForm *reco = new RecoWaveForm(waveForm, timeAxis, ch);

    hist001->Fill( fabs(reco->aReco()) );
    hist002->Fill( reco->tReco() );
    hist003->Fill( fabs(reco->amp(reco->imin())) );
    hist004->Fill( reco->pedMean() );
    hist005->Fill( reco->pedRMS() );
    hist006->Fill( reco->nRatios() );

    hist010->Fill( ntracks );
    if(ntracks>0 && chi2>1e-7){
      hist011->Fill( log10(chi2) );
      hist012->Fill( xIntercept, yIntercept );
      if( fabs(reco->aReco())>10.0*reco->pedRMS() ){
	hist013->Fill( xIntercept, yIntercept );
      }
    }

    // fill tree branches
    b_recoAmp = fabs(reco->aReco()) ;
    b_recoTime  = reco->tReco();
    b_maxSample = fabs(reco->amp(reco->imin()));
    b_pedMean   = reco->pedMean();
    b_pedRMS    = reco->pedRMS();
    b_nRatios   = reco->nRatios();
    b_nTracks   = ntracks;
    b_event     = jentry;

    if(ntracks>0 && chi2>1e-7){
      b_log10chi2 = log10(chi2) ;
      //hist012->Fill( xIntercept, yIntercept );
      //if( fabs(reco->aReco())>10.0*reco->pedRMS() ){
      //  hist013->Fill( xIntercept, yIntercept );
      //}
    }
    else
      b_log10chi2 = -99;
    
    
    tOut->Fill();
  }

  //TFile *fout = new TFile("simpleCheck.root","recreate");
  hist001->Write();
  hist002->Write();
  hist003->Write();
  hist004->Write();
  hist005->Write();
  hist006->Write();
  hist010->Write();
  hist011->Write();
  hist012->Write();
  hist013->Write();
  tOut->Write();
  //fout->Close();
}





void pulse::CalibrateOneChannel(int channelID=10)
{
  if (fChain == 0) return;
  Long64_t nentries = fChain->GetEntries();
  Long64_t nbytes = 0, nb = 0;


  int    dN = 3;
  double dT = 0.2;
  double parT[5] = { -1.20339, 3.94932, -4.77487, 5.69944, -0.630818 };
  double parA[5] = { 0, 0.0330576, 0.292408, 0.651366, 0.000751415 };
  ChannelDRS *ch = new ChannelDRS(5, parT, 5, parA, 0.1, 0.4, 0.1, dT, dN );
  

  // First iteration: find better parameters for time
  // reconstruction. This can be done on clipped waveforms as
  // well. Limit this step to 1000 good waveforms

  TGraphErrors *grRatios = new TGraphErrors();
  grRatios->SetName("grRatios");
  grRatios->SetTitle("T(R) before 1st iteration");
  grRatios->GetYaxis()->SetName("Ratio_{i}");
  grRatios->GetXaxis()->SetName("T_{i} - T_{0} (ns)");
  int ngr = 0;
  int nWFs = 0;

  for (Long64_t jentry=0; jentry<nentries;jentry++) {
    
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    if (jentry % 100 == 0) cout << "Processing Event " << jentry << " of " << nentries << "\n";
    nb = fChain->GetEntry(jentry);   nbytes += nb;

    double waveForm[1024];
    double timeAxis[1024];
    int group = channelID / 8;
    for(int is=0; is<1024; is++){
      waveForm[is] = channel[channelID][is];
      timeAxis[is] = time[group][is];
    }
    
    RecoWaveForm *reco = new RecoWaveForm(waveForm, timeAxis, ch);

    if(reco->amp(reco->imin())<-200.
       && reco->pedRMS()<5.0 && reco->tReco() < 200.0){
      if(nWFs>2000) break;
      nWFs++;
      for(int ip=0; ip<reco->nRatios(); ip++){
	grRatios->SetPoint(ngr, reco->ratioValue(ip), reco->timeValue(ip) - reco->tReco());
	grRatios->SetPointError(ngr, reco->ratioError(ip), 0.025);
	ngr++;
      }
    }
      
  }

  TF1 *ffit = new TF1("ffit","pol4",0.0, 0.8);
  for(int i=0; i<5; i++){
    ffit->SetParameter(i,parT[i]);
  }
  grRatios->Fit("ffit","W","",0.0,0.8);
  for(int i=0; i<5; i++){
    parT[i] = ffit->GetParameter(i);
  }
  cout << " new calibration for T(R): " << endl;
  cout << " parT[5] = { ";
  for(int i=0; i<4; i++)
    cout << parT[i] << ", ";
  cout << parT[4] << " };" << endl;

  // Apply new calibration

  for(int i=0; i<5; i++){
    ch->SetParTvsR(i, parT[i]);
  }

  // Second iteration: average pulse shape A(T). We need unclipped
  // waveforms for this

  TProfile *hAT = new TProfile("hAT", "average A(T)", 500, -50, 50);
  TH1D *hpedRMS = new TH1D("hpedRMS", "noise RMS; rms (adc)", 100, 0., 10.);
  
  for (Long64_t jentry=0; jentry<nentries;jentry++) {
    
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    if (jentry % 100 == 0) cout << "Processing Event " << jentry << " of " << nentries << "\n";
    nb = fChain->GetEntry(jentry);   nbytes += nb;

    double waveForm[1024];
    double timeAxis[1024];
    int group = channelID / 8;
    for(int is=0; is<1024; is++){
      waveForm[is] = channel[channelID][is];
      timeAxis[is] = time[group][is];
    }
    
    RecoWaveForm *reco = new RecoWaveForm(waveForm, timeAxis, ch);

    if(reco->amp(reco->imin())>-1800. && reco->amp(reco->imin())<-200.
       && reco->pedRMS()<5.0 && reco->tReco() < 200.0){
      hpedRMS->Fill( reco->pedRMS() );
      for(int ip=0; ip<1024; ip++){
	hAT->Fill( reco->tStart() + ip * ch->timeStep() - reco->tReco(), reco->amp(ip) / reco->amp(reco->imin()) ); 
      }
    }
      
  }

  // Analyze average pulse shape A(T)

  double ampMax = -1e+9;
  double timeMax = 0;
  for(int ib=1; ib<=hAT->GetNbinsX(); ib++){
    if(hAT->GetBinContent(ib)>ampMax){
      ampMax = hAT->GetBinContent(ib);
      timeMax = hAT->GetBinCenter(ib);
    }
  }


  TGraph *grAvsR = new TGraph();
  TGraph *grTvsR = new TGraph();
  ngr = 0;
  double tCurrent = -10.0;
  while(tCurrent<20.0){
    double a1 = hAT->Interpolate(tCurrent);
    double a2 = hAT->Interpolate(tCurrent + ch->ratioStep() * ch->timeStep());
    if( a1>1e-3 && a2>1e-3 && tCurrent<timeMax && a2<ampMax*0.8){
      double r = a1 / a2;
      grTvsR->SetPoint(ngr, r, tCurrent);
      grAvsR->SetPoint(ngr, r, a1 / ampMax);
      ngr++;
    }
    tCurrent += 0.1;
  }

  TF1 *ffitT = new TF1("ffitT", "pol4", 0., 1.);
  grTvsR->Fit("ffitT","W","",0.0, 0.8);
  grTvsR->SetName("grTvsR");
  grTvsR->SetTitle("T(R) after 1st iteration");
  grTvsR->SetMarkerStyle(20);
  
  TF1 *ffitA = new TF1("ffitA", "pol4", 0., 1.);
  ffitA->FixParameter(0,0);
  grAvsR->Fit("ffitA","W","",0.0, 0.8);
  grAvsR->SetName("grAvsR");
  grAvsR->SetTitle("A(R) after 1st iteration");
  grAvsR->SetMarkerStyle(20);

  for(int i=0; i<5; i++){
    parT[i] = ffitT->GetParameter(i);
    parA[i] = ffitA->GetParameter(i);
  }

  cout << " new calibration for T(R): " << endl;
  cout << " parT[5] = { ";
  for(int i=0; i<4; i++)
    cout << parT[i] << ", ";
  cout << parT[4] << " };" << endl;

  cout << " new calibration for A(R): " << endl;
  cout << " parA[5] = { ";
  for(int i=0; i<4; i++)
    cout << parA[i] << ", ";
  cout << parA[4] << " };" << endl;


  TFile *fout = new TFile("calibrate.root","recreate");
  grRatios->Write();
  hpedRMS->Write();
  hAT->Write();
  grTvsR->Write();
  grAvsR->Write();
  fout->Close();
}


