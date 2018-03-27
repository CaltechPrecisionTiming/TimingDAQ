//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Fri May 12 21:19:46 2017 by ROOT version 6.08/05
// from TTree T1037/The reconstructed telescope tracks
// found on file: Run502_CMSTiming_converted.root
//////////////////////////////////////////////////////////

''' HAVE to be deletated at the end'''

#ifndef FTBFPixelEvent_h
#define FTBFPixelEvent_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.

class FTBFPixelEvent {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   Double_t        event_xSlope;
   Double_t        event_ySlope;
   Double_t        event_xIntercept;
   Double_t        event_yIntercept;
   Double_t        event_chi2;
   Int_t           event_trigger;
   Int_t           event_runNumber;
   Long64_t        event_timestamp;

   // List of branches
   TBranch        *b_event;   //!

   FTBFPixelEvent(TTree *tree=0);
   virtual ~FTBFPixelEvent();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef FTBFPixelEvent_cxx
FTBFPixelEvent::FTBFPixelEvent(TTree *tree) : fChain(0)
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("Run502_CMSTiming_converted.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("Run502_CMSTiming_converted.root");
      }
      f->GetObject("T1037",tree);

   }
   Init(tree);
}

FTBFPixelEvent::~FTBFPixelEvent()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t FTBFPixelEvent::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t FTBFPixelEvent::LoadTree(Long64_t entry)
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

void FTBFPixelEvent::Init(TTree *tree)
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

   fChain->SetBranchAddress("event", &event_xSlope, &b_event);
   Notify();
}

Bool_t FTBFPixelEvent::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void FTBFPixelEvent::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t FTBFPixelEvent::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef FTBFPixelEvent_cxx
