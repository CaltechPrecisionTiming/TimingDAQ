#define TBAnalyse_cxx

#include <iostream>
#include <sstream>
#include <iomanip>
#include <TH2.h>
#include <TGraph.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TProfile.h>
#include <TMath.h>
#include <TComplex.h>
#include "TBAnalyse.hh"
#include "DrawScope.hh"
#include "Interpolator.hh"
#include "DrawInterpolation.hh"
#include "TimeOverThreshold.hh"
#include "VMinimum.hh"
#include "Correction.hh"
#include "DelayedZeroCrossing.hh"
#include "FrequencySpectrum.hh"
#include "DrawDelayedZeroCrossing.hh"

void TBAnalyse::Loop()
{
  std::cout << "================ Starting analysis ============" << std::endl;
   bool dump = false;
   bool scopePlots = false;
   bool interpolationPlots = false;
   bool delayedZeroCrossingPlots = false;
   bool timeOverThresh = true;
   bool frequencySpectrum = true;

   unsigned maxEvents = 0; // 0 == Infinity

   double tMin[] = {400,400,400}; // time range for signal
   double tMax[] = {600,600,600}; //

	unsigned  count_hit0 = 0;
	unsigned  count_hit1 = 0;
	unsigned  count_hit2 = 0;

	unsigned count_hit01 = 0;
	unsigned count_hit12 = 0;
	unsigned count_hit20 = 0;

	unsigned count_hit012 = 0;

  /////////////////////////////////////////////////////////////////////////////////
	// Global variables
	///////////////////////////////////////////////////////////////////z//////////////
	Interpolator voltage;
  /////////////////////////////////////////////////////////////////////////////////
	// Book histograms
	///////////////////////////////////////////////////////////////////z//////////////

	std::string histFileName  = "TBAnalysis";
	TFile* histFile = new TFile((histFileName+".root").c_str(),"RECREATE");  // histogram file
	TH1D h_vMin[4];
	h_vMin[0] = TH1D("vMin[0]","vMin[0]",100, -200.,0.);
	h_vMin[1] = TH1D("vMin[1]","vMin[1]",100, -200.,0.);
	h_vMin[2] = TH1D("vMin[2]","vMin[2]",100, -200.,0.);
	h_vMin[3] = TH1D("vMin[3]","vMin[3]",100, -200.,0.);


  /*
    Tree to collect all results
  */

  const float commom_threshold = -1000.;

  float ch1Amp, ch2Amp;
  float ch1_t0, ch1_t1, ch2_t0, ch2_t1;
  TTree* t_tree = new TTree("t_tree", "Tree with Time Stamps");
  t_tree->Branch("ch1Amp", &ch1Amp, "ch1Amp/F");
  t_tree->Branch("ch2Amp", &ch2Amp, "ch2Amp/F");
  t_tree->Branch("ch1_t0", &ch1_t0, "ch1_t0/F");
  t_tree->Branch("ch1_t1", &ch1_t1, "ch1_t1/F");
  t_tree->Branch("ch2_t0", &ch2_t0, "ch2_t0/F");
  t_tree->Branch("ch2_t1", &ch2_t1, "ch2_t1/F");



	TH1D h_frequencySpectrum1("frequencySpectrum1","frequencySpectrum1", 500, 0., 5.);
	TH1D h_frequencySpectrum2("frequencySpectrum2","frequencySpectrum2", 500, 0., 5.);

	unsigned nChans = 1500;
	double lowEdge = -500.;
	double hiEdge =  500.;

  std::cout << "here 0" << std::endl;
	TH1D h_timeRef("timeRef","timeRef",nChans,lowEdge,hiEdge);
	TH1D h_ch0_t1("ch0_t1","ch0_t1",nChans,lowEdge,hiEdge);
	TH1D h_ch0_t2("ch0_t2","ch0_t2",nChans,lowEdge,hiEdge);
	TH1D h_ch0_meanTime("ch0_meanTime","ch0_meanTime",nChans,lowEdge,hiEdge);
	TH1D h_ch0_tot("ch0_tot","ch0_tot",nChans,-500,500);

	TH1D h_ch1_t1("ch1_t1","ch1_t1",nChans,lowEdge,hiEdge);
	TH1D h_ch1_t2("ch1_t2","ch1_t2",nChans,lowEdge,hiEdge);
	TH1D h_ch1_tBest("ch1_tBest","ch1_tBest",nChans,lowEdge,hiEdge);
	TH1D h_ch1_meanTime("ch1_meanTime","ch1_meanTime",nChans,lowEdge,hiEdge);
	TH1D h_ch1_meanTimeCorrected("ch1_meanTimeCorrected","ch1_meanTimeCorrected",nChans,lowEdge,hiEdge);
	TH1D h_ch1_tot("ch1_tot","ch1_tot",nChans,-500,500);
	TH1D h_ch1_cfd("ch1_cfd","ch1_cfd",nChans,lowEdge,hiEdge);
	TH1D h_ch1_ZC("ch1_ZC","ch1_ZC",nChans,lowEdge,hiEdge);


	TH2D h_ch1_meanTimeVsTot("ch1_meanTimeVsTot","ch1_meanTimeVsTot",nChans,0.,3.,nChans,lowEdge,hiEdge);
	TProfile h_ch1_meanTimeVsTotP("ch1_meanTimeVsTotP","ch1_meanTimeVsTotP",nChans/10,0.,3.,-1.65,-1.25);

	TH2D h_ch1_meanTimeCorrectedVsTot("ch1_meanTimeCorrectedVsTot","ch1_meanTimeCorrectedVsTot",nChans,0.,3.,nChans,lowEdge,hiEdge);
	TProfile h_ch1_meanTimeCorrectedVsTotP("ch1_meanTimeCorrectedVsTotP","ch1_meanTimeCorrectedVsTotP",nChans/10,0.,3.,-1.65,-1.25);


	TH1D h_ch2_t1("ch2_t1","ch2_t1",nChans,lowEdge,hiEdge);
	TH1D h_ch2_t2("ch2_t2","ch2_t2",nChans,lowEdge,hiEdge);
	TH1D h_ch2_tBest("ch2_tBest","ch2_tBest",nChans,lowEdge,hiEdge);
	TH1D h_ch2_meanTime("ch2_meanTime","ch2_meanTime",nChans,lowEdge,hiEdge);
	TH1D h_ch2_meanTimeCorrected("ch2_meanTimeCorrected","ch2_meanTimeCorrected",nChans,lowEdge,hiEdge);
	TH1D h_ch2_tot("ch2_tot","ch2_tot",nChans,0.,3.);
	TH1D h_ch2_cfd("ch2_cfd","ch2_cfd",nChans,lowEdge,hiEdge);
	TH1D h_ch2_ZC("ch2_ZC","ch2_ZC",nChans,lowEdge,hiEdge);

  TH1D h_t0_t1("t0_t1","t0_t1",nChans,lowEdge,hiEdge);

	TH2D h_ch2_meanTimeVsTot("ch2_meanTimeVsTot","ch2_meanTimeVsTot",nChans,0.,3.,nChans,lowEdge,hiEdge);
	TProfile h_ch2_meanTimeVsTotP("ch2_meanTimeVsTotP","ch2_meanTimeVsTotP",nChans/10,0.,3.,-2.,-1.6);

	TH2D h_ch2_meanTimeCorrectedVsTot("ch2_meanTimeCorrectedVsTot","ch2_meanTimeCorrectedVsTot",nChans,0.,3.,nChans,lowEdge,hiEdge);
	TProfile h_ch2_meanTimeCorrectedVsTotP("ch2_meanTimeCorrectedVsTotP","ch2_meanTimeCorrectedVsTotP",nChans/10,0.,3.,-2.,-1.6);

  std::cout << "here 1" << std::endl;
  Long64_t nentries = fChain->GetEntriesFast();
  std::cout << "here 2" << std::endl;
  std::cout << "Starting analysis. " << nentries << " events in file." << std::endl;

   unsigned nEvents = nentries;
   if((maxEvents != 0) && (maxEvents < nentries)) nEvents = maxEvents;

   std::cout << nEvents << " events will be analysed." << std::endl;

   Long64_t nbytes = 0, nb = 0;
   //for (Long64_t jentry=0; jentry<10;jentry++) {
   for (Long64_t jentry=0; jentry<nEvents;jentry++) {
    for(unsigned iCh = 0; iCh < 2; ++iCh)
    {
      for(unsigned i = 0; i < 82; i++)
      {
        channel[iCh][i] = 0.0;
      }
    }
     	 Long64_t ientry = LoadTree(jentry);
     	 if (ientry < 0) break;
     	 nb = fChain->GetEntry(jentry);   nbytes += nb;

     	 if( (jentry % 1000 == 0) && (jentry != 0) ) std::cout << " event " << jentry << "/" << nEvents << std::endl;

     	 unsigned nSamples = sizeof(time[0])/sizeof(time[0][0]);
       //std::std::cout << nSamples << std::std::endl;
     	 // add back the baseline

		Float_t chanCorr[2][nSamples];
		for(unsigned iCh = 0; iCh < 2; ++iCh)
    {
      for(unsigned i = 0; i < nSamples; i++)
      {
        chanCorr[iCh][i] = channel[iCh][i];
      }
    }


		if(dump){
    	 std::cout << "\nEVENT " << event << std::endl;
     	 for (unsigned iSample = 0; iSample != nSamples; ++iSample){
     	 std::cout << std::setw(5) << iSample
    	  	<< " " << std::setw(10) << time[0][iSample]
    	  	<< " " << std::setw(15) << channel[0][iSample]
    	  	<< " " << std::setw(15) << channel[1][iSample]
    	  	<< " " << std::setw(15) << channel[2][iSample]
    	  	<< " " << std::setw(15) << channel[3][iSample]
     	 	<< std::endl;
    	  }
      }

/*
for ( int i = 0; i < 42; i++ )
  {
    std::std::cout << event << " " << time[0][i] << " "  << channel[0][i] << std::std::endl;
  }
*/
      if(scopePlots)
      {
        std::stringstream ss;
        for( unsigned iCh = 0; iCh < 2; iCh++ )
        {
          ss.str("");
          ss << "scopePlots/ev_" << event << "_ch_" << iCh;
          std::string fileName = ss.str();
          DrawScope(nSamples,0,525,time[0],channel[iCh],ss.str());
        }
		}

		double vMin[4];
    	for(unsigned iCh = 0; iCh < 2; ++iCh){
    		vMin[iCh] = VMinimum(tMin[iCh], tMax[iCh], nSamples, time[0],channel[iCh], true);
    		h_vMin[iCh].Fill(vMin[iCh]);
    	}
      ch1Amp = vMin[0];
      ch2Amp = vMin[1];
    	//bool hit0 = (vMin[0] < -60.) && (vMin[0] > - 140.);
    	//bool hit1 = vMin[1] < -12.;
    	//bool hit2 = vMin[2] < -40.;

      bool hit0 = true;
    	bool hit1 = true;
    	bool hit2 = false;

    	bool hit[] = {hit0,hit1,hit2};


      const unsigned nInterpolatedSamples = 42*5;
      Float_t interpolatedTimes[2][nInterpolatedSamples];
      Float_t interpolatedVoltages[2][nInterpolatedSamples];

      for(unsigned iCh = 0; iCh < 2; iCh++)
      {
        voltage.init(nSamples,time[0][0], time[0][nSamples-1], channel[iCh]);
        double tStep = (time[0][nSamples-1] - time [0][0])/nInterpolatedSamples;
        for(unsigned iS = 0; iS != nInterpolatedSamples; ++ iS)
        {
          double t = time[0][0] + iS*tStep;
          interpolatedTimes[iCh][iS] = t;
          interpolatedVoltages[iCh][iS] = voltage.f(t);
        }
        if(!hit[iCh]) continue; // plot only channels with signal
        //Drawing interpolated pulsed
        if(interpolationPlots)
        {
          std::stringstream ss;
          ss.str("");
					ss << "interpolationPlotsCh" << std::setw(1) << iCh << "/ev_" << event << "_ch_" << iCh;
					std::string fileName = ss.str();
					DrawInterpolation(tMin[iCh],tMax[iCh],nSamples,time[0],channel[iCh]
						,nInterpolatedSamples, interpolatedTimes[iCh], interpolatedVoltages[iCh]
						,ss.str());
			}

		}


		// frequency spectrum
		if(hit0 && frequencySpectrum)
    {
			FrequencySpectrum(nSamples, tMin[0], tMax[0], time[0], chanCorr[0], &h_frequencySpectrum2);
			//std::cout << "FrequencySpectrum" << std::endl;
		}

		if(hit1 && frequencySpectrum){
			FrequencySpectrum(nSamples, tMin[1], tMax[1], time[0], chanCorr[1], &h_frequencySpectrum1);
			//std::cout << "FrequencySpectrum" << std::endl;
		}

    //-------------------------
		// find time over threshold
    //-------------------------
		if(timeOverThresh && hit0)
    {
      double t1[3],t2[3],tot[3],tCfd[3],tDummy,tZC[3];
      if(hit0)
      {
        int retCode = TimeOverThreshold( commom_threshold,tMin[0],tMax[0],nSamples, time[0], chanCorr[0], t1[0],t2[0]);
				if(retCode != 0)
        {
					//std::cout << "Evt: " << event << " channel[0] "<< "TimeOverThreshold error " << retCode << std::endl;
					hit0 = false;
				}
				tot[0]=t2[0]-t1[0];
				//std::cout << "evt: " << event << " t1[0]: " << t1[0] << " t2[0]: " << t2[0] <<  " tot: " << tot[0] << std::endl;
			}

      if(hit1)
      {
        int retCode = TimeOverThreshold(commom_threshold, tMin[1], tMax[1], nSamples, time[0], chanCorr[1], t1[1],t2[1]);
        if(retCode != 0)
        {
          //std::cout << "Evt: "  << event << " channel[1] "<< "TimeOverThreshold error " << retCode << std::endl;
          hit1 = false;
        }
        tot[1]=t2[1]-t1[1];
				//std::cout << "evt: " << event << " t1[1]: " << t1[1] << " t2[1]: " << t2[1] <<  " tot: " << tot[1] << std::endl;
			}

			if(hit1){
				// constant fraction discriminator	(20% of maximum)
				int retCodeCfd = TimeOverThreshold(vMin[1]*0.2,tMin[1],tMax[1],
						nSamples, time[0], chanCorr[1], tCfd[1],tDummy);
				if(retCodeCfd != 0) {
					std::cout << "Evt: "  << event << " channel[1] "<< "TimeCFD error " << retCodeCfd << std::endl;
					hit1 = false;
				}
			}

			if(hit1){
				// delayed zero crossing

				double ZCThresh = -10.;
				double ZCDelay = 0.45;
				double ZCGain = 3.;

				int retZeroCrossing = DelayedZeroCrossing(ZCThresh, ZCDelay, ZCGain,tMin[1],tMax[1],
						nSamples, time[0], chanCorr[1], tZC[1]);
				if(retZeroCrossing != 0) {
					std::cout << "Evt: "  << event << " channel[1] "<< "DelayedZeroCrossing error " << retZeroCrossing << std::endl;
					hit1 = false;
				}

				if(delayedZeroCrossingPlots){
					std::stringstream ss;
					ss.str("");
					ss << "delayedZeroCrossingPlotsCh1"  << "/ev_" << event << "_ch_1";
					std::string fileName = ss.str();
					DrawDelayedZeroCrossing(ZCDelay,ZCGain,tMin[1],tMax[1]
						,nSamples,time[0],channel[1]
						,nInterpolatedSamples, interpolatedTimes[1], interpolatedVoltages[1]
						,ss.str());
				}
			}


			if(hit2){

				int retCode = TimeOverThreshold(-30.,tMin[2],tMax[2],
						nSamples, time[0], chanCorr[2], t1[2],t2[2]);
				if(retCode != 0) {
					//std::cout << "Evt: "  << event << " channel[2] "<< "TimeOverThreshold error " << retCode << std::endl;
					hit2 = false;
				}
				tot[2]=t2[2]-t1[2];
				//std::cout << "evt: " << event << " t1[2]: " << t1[2] << " t2[2]: " << t2[2] <<  " tot: " << tot[2] << std::endl;
			}

			if(hit2){
				// constant fraction discriminator (20% of maximum)
				int retCodeCfd = TimeOverThreshold(vMin[2]*0.2,tMin[2],tMax[2],
						nSamples, time[0], chanCorr[2], tCfd[2],tDummy);
				if(retCodeCfd != 0) {
					std::cout << "Evt: "  << event << " channel[2] "<< "TimeCFD error " << retCodeCfd << std::endl;
					hit2 = false;
				}
			}

			if(hit2){
				// delayed zero crossing
				double ZCThresh = -30.;
				double ZCDelay = 0.55;
				double ZCGain = 6.0;
				int retZeroCrossing = DelayedZeroCrossing(ZCThresh, ZCDelay, ZCGain,tMin[2],tMax[2],
						nSamples, time[0], chanCorr[2], tZC[2]);
				if(retZeroCrossing != 0) {
					std::cout << "Evt: "  << event << " channel[2] "<< "DelayedZeroCrossing error " << retZeroCrossing << std::endl;
					hit2 = false;
				}

				if(delayedZeroCrossingPlots){
					std::stringstream ss;
					ss.str("");
					ss << "delayedZeroCrossingPlotsCh2"  << "/ev_" << event << "_ch_2";
					std::string fileName = ss.str();
					DrawDelayedZeroCrossing(ZCDelay,ZCGain,tMin[2],tMax[2]
						,nSamples,time[0],channel[2]
						,nInterpolatedSamples, interpolatedTimes[2], interpolatedVoltages[2]
						,ss.str());
				}
			}



			double timeRef = 0;
			//double timeRef = ch0_meanTime;
			h_timeRef.Fill(timeRef);

      //Fill tree variables
      ch1_t0 = t1[0];
      ch1_t1 = t2[0];
      ch2_t0 = t1[1];
      ch2_t1 = t2[1];
      t_tree->Fill();

      //std::std::cout << "hits " << hit0 << " " << hit1 << " " << hit2 << std::std::endl;
      hit0 = true;
      hit1 = true;
			double ch0_t1 = t1[0]-timeRef ;
			double ch0_t2 = t2[0]-timeRef;
			double ch0_meanTime = (ch0_t1 + ch0_t2)/2. -timeRef;
			double ch0_tot = (ch0_t2 - ch0_t1)/2.;


			double ch1_t1 = t1[1]-timeRef;
			double ch1_t2 = t2[1]-timeRef;
			double ch1_tBest = LP2_20[1]-timeRef;
			double ch1_meanTime = (ch1_t1 + ch1_t2)/2.;
			double ch1_tot = (ch1_t2 - ch1_t1)/2.;
			double ch1_cfd = tCfd[1] - timeRef;
			double ch1_ZC = tZC[1] - timeRef;
			double ch1_meanTimeCorrected = ch1_meanTime - Correction(1,ch1_tot);


			double ch2_t1 = t1[2]-timeRef;
			double ch2_t2 = t2[2]-timeRef;
			double ch2_tBest = LP2_20[2]-timeRef;
			double ch2_meanTime = (ch2_t1 + ch2_t2)/2.;
			double ch2_tot = (ch2_t2 - ch2_t1)/2.;
			double ch2_cfd = tCfd[2] - timeRef;
			double ch2_ZC = tZC[2] - timeRef;
			double ch2_meanTimeCorrected = ch2_meanTime - Correction(2, ch2_tot);


			if(hit0){
				h_ch0_t1.Fill(ch0_t1);
				h_ch0_t2.Fill(ch0_t2);
				h_ch0_meanTime.Fill(ch0_meanTime);
				h_ch0_tot.Fill(ch0_tot);


				if(hit1){
				//if(true){
					h_ch1_t1.Fill(ch1_t1);
					h_ch1_t2.Fill(ch1_t2);
					h_ch1_tBest.Fill(ch1_tBest);
					h_ch1_meanTime.Fill(ch1_meanTime);
					h_ch1_meanTimeCorrected.Fill(ch1_meanTimeCorrected);
					h_ch1_tot.Fill(ch1_tot);
					h_ch1_cfd.Fill(ch1_cfd);
					h_ch1_ZC.Fill(ch1_ZC);

					h_ch1_meanTimeVsTot.Fill(ch1_tot,ch1_meanTime);
					h_ch1_meanTimeVsTotP.Fill(ch1_tot,ch1_meanTime);
					h_ch1_meanTimeCorrectedVsTot.Fill(ch1_tot,ch1_meanTimeCorrected);
					h_ch1_meanTimeCorrectedVsTotP.Fill(ch1_tot,ch1_meanTimeCorrected);
          h_t0_t1.Fill(ch1_t1-ch0_t1);
				}

				if(hit2){
				//if(true){
					h_ch2_t1.Fill(ch2_t1);
					h_ch2_t2.Fill(ch2_t2);
					h_ch2_tBest.Fill(ch2_tBest);
					h_ch2_meanTime.Fill(ch2_meanTime);
					h_ch2_meanTimeCorrected.Fill(ch2_meanTimeCorrected);
					h_ch2_tot.Fill(ch2_tot);
					h_ch2_cfd.Fill(ch2_cfd);
					h_ch2_ZC.Fill(ch2_ZC);

					h_ch2_meanTimeVsTot.Fill(ch2_tot,ch2_meanTime);
					h_ch2_meanTimeVsTotP.Fill(ch2_tot,ch2_meanTime);
					h_ch2_meanTimeCorrectedVsTot.Fill(ch2_tot,ch2_meanTimeCorrected);
					h_ch2_meanTimeCorrectedVsTotP.Fill(ch2_tot,ch2_meanTimeCorrected);
				}
			}


		}  // end time over threshold



        if(hit0) ++count_hit0;
        if(hit1) ++count_hit1;
        if(hit2) ++count_hit2;

        if(hit0 && hit1) ++count_hit01;
        if(hit1 && hit2) ++count_hit12;
        if(hit2 && hit0) ++count_hit20;

        if(hit0 && hit1 && hit2) ++count_hit012;


    } // end loop on entries

    std::cout << "count_hit0 = " << count_hit0 << std::endl;
    std::cout << "count_hit1 = " << count_hit1 << std::endl;
    std::cout << "count_hit2 = " << count_hit2 << std::endl;


    std::cout << "count_hit01 = " << count_hit01 << std::endl;
    std::cout << "count_hit12 = " << count_hit12 << std::endl;
    std::cout << "count_hit20 = " << count_hit20 << std::endl;

    std::cout << "count_hit012 = " << count_hit012 << std::endl;


    histFile->Write();

    //delete IntScopeCanvas;

} // TBAnalyse
