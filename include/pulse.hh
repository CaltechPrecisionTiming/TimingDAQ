//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Mon Jun  5 22:54:41 2017 by ROOT version 6.08/05
// from TTree pulse/Digitized waveforms
// found on file: data/RECO/v2/Run531_RECO.root
//////////////////////////////////////////////////////////

#ifndef pulse_h
#define pulse_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TString.h>

// Header file for the classes stored in the TTree if any.

class pulse {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain
  
   TTree          *tOut; //output TTree
   // Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   Int_t           event;
   UShort_t        tc[4];
   Short_t         channel[36][1024];
   Double_t        channelFilter[36][1024];
   Float_t         time[4][1024];
   Float_t         xmin[36];
   Float_t         amp[36];
   Float_t         base[36];
   Float_t         integral[36];
   Float_t         intfull[36];
   Float_t         gauspeak[36];
   Float_t         sigmoidTime[36];
   Float_t         fullFitTime[36];
   Float_t         linearTime0[36];
   Float_t         linearTime15[36];
   Float_t         linearTime30[36];
   Float_t         linearTime45[36];
   Float_t         linearTime60[36];
   Float_t         fallingTime[36];
   Float_t         risetime[36];
   Float_t         constantThresholdTime[36];
   Bool_t          isRinging[36];
   Float_t         xIntercept;
   Float_t         yIntercept;
   Float_t         xSlope;
   Float_t         ySlope;
   Float_t         x1;
   Float_t         y1;
   Float_t         x2;
   Float_t         y2;
   Float_t         chi2;
   Int_t           ntracks;

   // List of branches
   TBranch        *b_event;   //!
   TBranch        *b_tc;   //!
   TBranch        *b_channel;   //!
   TBranch        *b_channelFilter;   //!
   TBranch        *b_time;   //!
   TBranch        *b_xmin;   //!
   TBranch        *b_amp;   //!
   TBranch        *b_base;   //!
   TBranch        *b_integral;   //!
   TBranch        *b_intfull;   //!
   TBranch        *b_gauspeak;   //!
   TBranch        *b_sigmoidTime;   //!
   TBranch        *b_fullFitTime;   //!
   TBranch        *b_linearTime0;   //!
   TBranch        *b_linearTime15;   //!
   TBranch        *b_linearTime30;   //!
   TBranch        *b_linearTime45;   //!
   TBranch        *b_linearTime60;   //!
   TBranch        *b_fallingTime;   //!
   TBranch        *b_risetime;   //!
   TBranch        *b_constantThresholdTime;   //!
   TBranch        *b_isRinging;   //!
   TBranch        *b_xIntercept;   //!
   TBranch        *b_yIntercept;   //!
   TBranch        *b_xSlope;   //!
   TBranch        *b_ySlope;   //!
   TBranch        *b_x1;   //!
   TBranch        *b_y1;   //!
   TBranch        *b_x2;   //!
   TBranch        *b_y2;   //!
   TBranch        *b_chi2;   //!
   TBranch        *b_ntracks;   //!

   pulse(TTree *tree=0);
   virtual ~pulse();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);

  void SimpleCheck(int channelID, std::string channelName);
  void CalibrateOneChannel(int channelID);
  TTree* GetOutputTree();
};

#endif

#ifdef pulse_cxx
pulse::pulse(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("data/RECO/v2/Run531_RECO.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("data/RECO/v2/Run531_RECO.root");
      }
      f->GetObject("pulse",tree);

   }
   Init(tree);
}

pulse::~pulse()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t pulse::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t pulse::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void pulse::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("i_evt", &event, &b_event);
   fChain->SetBranchAddress("tc", tc, &b_tc);
   fChain->SetBranchAddress("channel", channel, &b_channel);
   fChain->SetBranchAddress("channelFilter", channelFilter, &b_channelFilter);
   fChain->SetBranchAddress("time", time, &b_time);
   //fChain->SetBranchAddress("xmin", xmin, &b_xmin);
   fChain->SetBranchAddress("t_peak", xmin, &b_xmin);
   fChain->SetBranchAddress("amp", amp, &b_amp);
   //fChain->SetBranchAddress("V_peak", amp, &b_amp);
   //fChain->SetBranchAddress("base", base, &b_base);
   fChain->SetBranchAddress("baseline", base, &b_base);
   fChain->SetBranchAddress("integral", integral, &b_integral);
   fChain->SetBranchAddress("intfull", intfull, &b_intfull);
   //fChain->SetBranchAddress("gauspeak", gauspeak, &b_gauspeak);
   fChain->SetBranchAddress("gaus_mean", gauspeak, &b_gauspeak);
   fChain->SetBranchAddress("sigmoidTime", sigmoidTime, &b_sigmoidTime);
   fChain->SetBranchAddress("fullFitTime", fullFitTime, &b_fullFitTime);
   fChain->SetBranchAddress("linearTime0", linearTime0, &b_linearTime0);
   fChain->SetBranchAddress("linearTime15", linearTime15, &b_linearTime15);
   fChain->SetBranchAddress("linearTime30", linearTime30, &b_linearTime30);
   fChain->SetBranchAddress("linearTime45", linearTime45, &b_linearTime45);
   fChain->SetBranchAddress("linearTime60", linearTime60, &b_linearTime60);
   //fChain->SetBranchAddress("fallingTime", fallingTime, &b_fallingTime);
   fChain->SetBranchAddress("fallingtime", fallingTime, &b_fallingTime);
   fChain->SetBranchAddress("risetime", risetime, &b_risetime);
   fChain->SetBranchAddress("constantThresholdTime", constantThresholdTime, &b_constantThresholdTime);
   fChain->SetBranchAddress("isRinging", isRinging, &b_isRinging);
   fChain->SetBranchAddress("xIntercept", &xIntercept, &b_xIntercept);
   fChain->SetBranchAddress("yIntercept", &yIntercept, &b_yIntercept);
   fChain->SetBranchAddress("xSlope", &xSlope, &b_xSlope);
   fChain->SetBranchAddress("ySlope", &ySlope, &b_ySlope);
   fChain->SetBranchAddress("x1", &x1, &b_x1);
   fChain->SetBranchAddress("y1", &y1, &b_y1);
   fChain->SetBranchAddress("x2", &x2, &b_x2);
   fChain->SetBranchAddress("y2", &y2, &b_y2);
   fChain->SetBranchAddress("chi2", &chi2, &b_chi2);
   fChain->SetBranchAddress("ntracks", &ntracks, &b_ntracks);
   Notify();
}

Bool_t pulse::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void pulse::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t pulse::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef pulse_cxx
