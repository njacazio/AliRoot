/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/*
$Log$
*/


// Event generator that using an instance of type AliGenReader
// reads particles from a file and applies cuts. 


#include "AliGenBeamGas.h"
#include "AliRun.h"

#include <TParticle.h>
#include <TFile.h>
#include <TTree.h>


 ClassImp(AliGenBeamGas)

AliGenBeamGas::AliGenBeamGas()
  :AliGenExtFile()
{
//  Constructor
//
    fInteractions = 1;
}

AliGenBeamGas::AliGenBeamGas(const AliGenBeamGas & ExtFile)
{
// copy constructor
}
//____________________________________________________________

AliGenBeamGas::~AliGenBeamGas()
{
// Destructor
    delete fReader;
}

//___________________________________________________________
void AliGenBeamGas::Init()
{
// Initialize
    AliGenExtFile::Init();
}

    
void AliGenBeamGas::Generate()
{
// Generate particles

  Float_t polar[3]  = {0,0,0};
  Float_t origin[3] = {0,0,0};
  Float_t p[3];
  Float_t random[2];
  Int_t i, nt;
  Int_t nInt = 0;
  
  while(nInt < fInteractions) {
//
      Rndm(random,2);
//
//  Interaction vertex
//
      origin[2] = 4000. * random[0] - 2000.;
//
//    beam 1 or 2
//      
      Float_t ibeam = (random[1] < 0.5) ? -1. : 1.;
      
//
//    Read next event
//      
      Int_t nTracks = fReader->NextEvent(); 	
      if (nTracks == 0) {
	  // printf("\n No more events !!! !\n");
	  Warning("AliGenBeamGas::Generate",
		  "\nNo more events in external file!!!\n Last event may be empty or incomplete.\n");
	  return;
      }
      
      //
      // Stack filling loop
      //
      for (i = 0; i < nTracks; i++) {
	  TParticle* iparticle = fReader->NextParticle();
	  p[0] = iparticle->Px();
	  p[1] = iparticle->Py();
	  p[2] = iparticle->Pz() * ibeam;
	
	  Int_t idpart     = iparticle->GetPdgCode();
	  Int_t decayed    = iparticle->GetFirstDaughter();
	  Int_t doTracking = fTrackIt && (decayed < 0) && (TMath::Abs(idpart) > 10);
	  SetTrack(doTracking,-1,idpart,p,origin,polar,0,kPPrimary,nt);
	  KeepTrack(nt);
      } // track loop
      nInt++;
  } // event loop
//
  SetHighWaterMark(nt);
//
  CdEventFile();
}


//AliGenBeamGas& AliGenBeamGas::operator=(const  AliGenBeamGas& rhs)
//{
// Assignment operator
//    return *this;
//}








