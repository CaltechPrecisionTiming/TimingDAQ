#include <iostream>
#include <string>
#include "TTree.h"
#include "TFile.h"
#include "TList.h"
#include "pulse.hh"

std::string ParseCommandLine( int argc, char* argv[], std::string opt )
{
  for (int i = 1; i < argc; i++ )
    {
      std::string tmp( argv[i] );
      if ( tmp.find( opt ) != std::string::npos )
	{
	  if ( tmp.find( "=" )  != std::string::npos ) return tmp.substr( tmp.find_last_of("=") + 1 );
	  if ( tmp.find( "--" ) != std::string::npos ) return "yes";
	}
    }
  
  return "";
};


int main( int argc, char** argv)
{

  std::string inputRootFile = ParseCommandLine( argc, argv, "--inputRootFile" );
  if ( inputRootFile == "" )
    {
      std::cerr << "[ERROR]: Please provide a valid ROOT file to run on!! Please use --inputRootFile=<your_file_name>\n";
      return -1;
    }

  bool mergeChannels = false;
  std::string s_mergeChannels = ParseCommandLine( argc, argv, "--mergeChannels" );

  if ( s_mergeChannels == "true" || s_mergeChannels == "True")
    mergeChannels = true;
  else if ( s_mergeChannels == "false" || s_mergeChannels == "False" || s_mergeChannels == "") 
    mergeChannels = false;
  else {
    std::cerr << "[ERROR]: Please use --mergeChannels=<true/false> . Running program without specifying option defaults to false\n";
    return -1;
  }

  TFile* fIn = new TFile(inputRootFile.c_str(), "READ");
  TTree* myTree = (TTree*)fIn->Get("pulse");
  pulse* myPulse = new pulse( myTree );
  TFile *fOut = new TFile(inputRootFile.c_str(),"UPDATE");
  //TFile *fOut = new TFile("simpleCheck.root","recreate");


  // Check performance of one DRS channel. Few simple distributoins
  myPulse->SimpleCheck(10, "ch10");
  TTree* t_chA = (TTree*)myPulse->GetOutputTree();
  myPulse->SimpleCheck(11, "ch11");
  TTree* t_chB = (TTree*)myPulse->GetOutputTree();


  // Calibrate Ratios for one channel
  myPulse->CalibrateOneChannel(10);
  myPulse->CalibrateOneChannel(11);


  // produced merged tree if requested
  std::cout << "mergeChannels: " << mergeChannels << std::endl;
  if (mergeChannels) {
    std::cout << "t_chA->GetName(): " << t_chA->GetName() << "\tt_chA->GetEntries(): " << t_chA->GetEntries() << std::endl;
    std::cout << "t_chB->GetName(): " << t_chB->GetName() << "\tt_chB->GetEntries(): " << t_chB->GetEntries() << std::endl;
    
    double tA_event, tB_event;
    double tA_recoTime, tB_recoTime;
    double tA_recoAmp, tB_recoAmp;
    t_chB->BuildIndex("event");
    t_chA->SetBranchAddress("event", &tA_event);
    t_chA->SetBranchAddress("recoTime_ch10", &tA_recoTime);
    t_chA->SetBranchAddress("recoAmp_ch10", &tA_recoAmp);
    t_chB->SetBranchAddress("event", &tB_event);
    t_chB->SetBranchAddress("recoTime_ch11", &tB_recoTime);
    t_chB->SetBranchAddress("recoAmp_ch11", &tB_recoAmp);
    t_chA->AddFriend(t_chB);

    TTree* mergeTree = new TTree("mergeTree", "mergeTree");
    mergeTree->Branch("event", &tA_event);
    mergeTree->Branch("recoTime_ch10", &tA_recoTime);
    mergeTree->Branch("recoAmp_ch10", &tA_recoAmp);
    mergeTree->Branch("recoTime_ch11", &tB_recoTime);
    mergeTree->Branch("recoAmp_ch11", &tB_recoAmp);

    Long64_t nentries = t_chA->GetEntries();
    Int_t nSame = 0;
    for (Long64_t i=0;i<nentries;i++) {
      t_chA->GetEntry(i);
      if ( tA_event==tB_event) {
	nSame++;
	mergeTree->Fill();
      } 
      else {
	if (t_chA->GetEntryWithIndex(tA_event) > 0) 
	  std::cout << "Problem with events in one channel but not another!!!!!" << std::endl;
      }
    }
    printf("nSame = %d, fentries=%lld\n",nSame,t_chA->GetEntries());
    mergeTree->Write();
    
    /*TList* list = new TList();
    TTree* tClone_A = (TTree*)t_chA->Clone();
    TTree* tClone_B = (TTree*)t_chB->Clone();
    gROOT->cd(); // new
    list->Add(tClone_A);
    list->Add(tClone_B);
    TTree *mergeTree = TTree::MergeTrees(list); 
    mergeTree->SetName("mergeTree");
    fOut->cd();
    std::cout << "mergeTree->GetName(): " << mergeTree->GetName() << "\tmergeTree->GetEntries(): " << mergeTree->GetEntries() << std::endl;
    mergeTree->Write();
    */

  }

  fOut->Close();

  return 0;
}
