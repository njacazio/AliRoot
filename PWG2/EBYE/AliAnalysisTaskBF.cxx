#include "TChain.h"
#include "TList.h"
#include "TCanvas.h"
#include "TLorentzVector.h"
#include "TGraphErrors.h"
#include "TH1F.h"

#include "AliAnalysisTaskSE.h"
#include "AliAnalysisManager.h"

#include "AliESDVertex.h"
#include "AliESDEvent.h"
#include "AliESDInputHandler.h"
#include "AliAODEvent.h"
#include "AliAODTrack.h"
#include "AliAODInputHandler.h"
#include "AliMCEventHandler.h"
#include "AliMCEvent.h"
#include "AliStack.h"
#include "AliESDtrackCuts.h"

#include "AliBalance.h"

#include "AliAnalysisTaskBF.h"

// Analysis task for the BF code
// Authors: Panos Cristakoglou@cern.ch

ClassImp(AliAnalysisTaskBF)

//________________________________________________________________________
AliAnalysisTaskBF::AliAnalysisTaskBF(const char *name) 
  : AliAnalysisTaskSE(name), 
    fBalance(0),
    fList(0),
    fHistEventStats(0),
    fESDtrackCuts(0) {
  // Constructor

  // Define input and output slots here
  // Input slot #0 works with a TChain
  DefineInput(0, TChain::Class());
  // Output slot #0 writes into a TH1 container
  DefineOutput(1, AliBalance::Class());
  DefineOutput(2, TList::Class());
}

//________________________________________________________________________
void AliAnalysisTaskBF::UserCreateOutputObjects() {
  // Create histograms
  // Called once
  if(!fBalance) {
    fBalance = new AliBalance();
    fBalance->SetAnalysisLevel("ESD");
    fBalance->SetAnalysisType(1);
    fBalance->SetNumberOfBins(18);
    fBalance->SetInterval(-0.9,0.9);
  }

  fList = new TList();
  fList->SetName("listQA");

  TString gCutName[4] = {"Total","Offline trigger",
                         "Vertex","Analyzed"};
  fHistEventStats = new TH1F("fHistEventStats",
                             "Event statistics;;N_{events}",
                             4,0.5,4.5);
  for(Int_t i = 1; i <= 4; i++)
    fHistEventStats->GetXaxis()->SetBinLabel(i,gCutName[i-1].Data());
  fList->Add(fHistEventStats);

  if(fESDtrackCuts) fList->Add(fESDtrackCuts);

  // Post output data.
  PostData(1, fBalance);
  PostData(2, fList);
}

//________________________________________________________________________
void AliAnalysisTaskBF::UserExec(Option_t *) {
  // Main loop
  // Called for each event
  TString gAnalysisLevel = fBalance->GetAnalysisLevel();

  TObjArray *array = new TObjArray();
  
  //ESD analysis
  if(gAnalysisLevel == "ESD") {
    AliESDEvent* gESD = dynamic_cast<AliESDEvent*>(InputEvent()); // from TaskSE
    if (!gESD) {
      Printf("ERROR: gESD not available");
      return;
    }

    fHistEventStats->Fill(1); //all events
    Bool_t isSelected = ((AliInputEventHandler*)(AliAnalysisManager::GetAnalysisManager()->GetInputEventHandler()))->IsEventSelected();
    if(isSelected) {
      fHistEventStats->Fill(2); //triggered events

      const AliESDVertex *vertex = gESD->GetPrimaryVertex();
      if(vertex) {
	fHistEventStats->Fill(3); //events with a proper vertex
	fHistEventStats->Fill(4); //analayzed events
	
	Printf("There are %d tracks in this event", gESD->GetNumberOfTracks());
	for (Int_t iTracks = 0; iTracks < gESD->GetNumberOfTracks(); iTracks++) {
	  AliESDtrack* track = gESD->GetTrack(iTracks);
	  if (!track) {
	    Printf("ERROR: Could not receive track %d", iTracks);
	    continue;
	  }

	  //ESD track cuts
	  if(fESDtrackCuts) 
	    if(!fESDtrackCuts->AcceptTrack(track)) continue;
	  array->Add(track);
	} //track loop
      }//vertex object valid
    }//triggered event 
  }//ESD analysis
  //AOD analysis
  else if(gAnalysisLevel == "AOD") {
    AliAODEvent* gAOD = dynamic_cast<AliAODEvent*>(InputEvent()); // from TaskSE
    if(!gAOD) {
      Printf("ERROR: gAOD not available");
      return;
    }

    Printf("There are %d tracks in this event", gAOD->GetNumberOfTracks());
    for (Int_t iTracks = 0; iTracks < gAOD->GetNumberOfTracks(); iTracks++) {
      AliAODTrack* track = gAOD->GetTrack(iTracks);
      if (!track) {
	Printf("ERROR: Could not receive track %d", iTracks);
	continue;
      }
      array->Add(track);
    } //track loop
  }//AOD analysis
  //MC analysis
  else if(gAnalysisLevel == "MC") {

    AliMCEvent*  mcEvent = MCEvent(); 
    if (!mcEvent) {
      Printf("ERROR: mcEvent not available");
      return;
    }
    
    Printf("There are %d tracks in this event", mcEvent->GetNumberOfPrimaries());
    for (Int_t iTracks = 0; iTracks < mcEvent->GetNumberOfPrimaries(); iTracks++) {
      AliMCParticle* track = dynamic_cast<AliMCParticle *>(mcEvent->GetTrack(iTracks));
      if (!track) {
	Printf("ERROR: Could not receive particle %d", iTracks);
	continue;
      }
      array->Add(track);
    } //track loop
  }//MC analysis

  fBalance->CalculateBalance(array);
  
  delete array;
  
}      

//________________________________________________________________________
void AliAnalysisTaskBF::Terminate(Option_t *) {
  // Draw result to the screen
  // Called once at the end of the query
  fBalance = dynamic_cast<AliBalance*> (GetOutputData(1));
  if (!fBalance) {
    Printf("ERROR: fBalance not available");
    return;
  }
  
  TGraphErrors *gr = fBalance->DrawBalance();
  gr->SetMarkerStyle(20);
  gr->Draw("AP");

  fBalance->PrintResults();
}
