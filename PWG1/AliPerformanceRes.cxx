//------------------------------------------------------------------------------
// Implementation of AliPerformanceRes class. It keeps information from 
// comparison of reconstructed and MC particle tracks. In addtion, 
// it keeps selection cuts used during comparison. The comparison 
// information is stored in the ROOT histograms. Analysis of these 
// histograms can be done by using Analyse() class function. The result of 
// the analysis (histograms/graphs) are stored in the folder which is
// a data member of AliPerformanceRes.
//
// Author: J.Otwinowski 04/02/2008 
//------------------------------------------------------------------------------

/*
 
  // after running comparison task, read the file, and get component
  gROOT->LoadMacro("$ALICE_ROOT/PWG1/Macros/LoadMyLibs.C");
  LoadMyLibs();

  TFile f("Output.root");
  //AliPerformanceRes * compObj = (AliPerformanceRes*)f.Get("AliPerformanceRes");
  AliPerformanceRes * compObj = (AliPerformanceRes*)cOutput->FindObject("AliPerformanceRes");
 
  // analyse comparison data
  compObj->Analyse();

  // the output histograms/graphs will be stored in the folder "folderRes" 
  compObj->GetAnalysisFolder()->ls("*");

  // user can save whole comparison object (or only folder with anlysed histograms) 
  // in the seperate output file (e.g.)
  TFile fout("Analysed_Res.root","recreate");
  compObj->Write(); // compObj->GetAnalysisFolder()->Write();
  fout.Close();

*/

#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TAxis.h"
#include "TF1.h"

#include "AliPerformanceRes.h" 
#include "AliESDEvent.h" 
#include "AliESDVertex.h"
#include "AliESDtrack.h"
#include "AliLog.h" 
#include "AliMCEvent.h" 
#include "AliMCParticle.h" 
#include "AliHeader.h" 
#include "AliGenEventHeader.h" 
#include "AliStack.h" 
#include "AliMCInfoCuts.h" 
#include "AliRecInfoCuts.h" 
#include "AliTracker.h" 
#include "AliTreeDraw.h" 

using namespace std;

ClassImp(AliPerformanceRes)

//_____________________________________________________________________________
AliPerformanceRes::AliPerformanceRes():
  AliPerformanceObject("AliPerformanceRes"),
  fResolHisto(0),
  fPullHisto(0),

  // Cuts 
  fCutsRC(0),  
  fCutsMC(0),  

  // histogram folder 
  fAnalysisFolder(0)
{
  Init();
}

//_____________________________________________________________________________
AliPerformanceRes::AliPerformanceRes(Char_t* name="AliPerformanceRes", Char_t* title="AliPerformanceRes",Int_t analysisMode=0,Bool_t hptGenerator=kFALSE):
  AliPerformanceObject(name,title),
  fResolHisto(0),
  fPullHisto(0),

  // Cuts 
  fCutsRC(0),  
  fCutsMC(0),  

  // histogram folder 
  fAnalysisFolder(0)
{
  // named constructor	
  // 
  SetAnalysisMode(analysisMode);
  SetHptGenerator(hptGenerator);

  Init();
}

//_____________________________________________________________________________
AliPerformanceRes::~AliPerformanceRes()
{
  // destructor
   
  if(fResolHisto) delete fResolHisto; fResolHisto=0;     
  if(fPullHisto)  delete fPullHisto;  fPullHisto=0;     
  
  if(fAnalysisFolder) delete fAnalysisFolder; fAnalysisFolder=0;
}

//_____________________________________________________________________________
void AliPerformanceRes::Init(){

  //
  // histogram bining
  //

  // set pt bins
  Int_t nPtBins = 50;
  Double_t ptMin = 1.e-2, ptMax = 10.;

  Double_t *binsPt = 0;
  if (IsHptGenerator())  { 
    nPtBins = 100; ptMax = 100.;
    binsPt = CreateLogAxis(nPtBins,ptMin,ptMax);
  } else {
    binsPt = CreateLogAxis(nPtBins,ptMin,ptMax);
  }

  /*
  Int_t nPtBins = 31;
  Double_t binsPt[32] = {0.,0.05,0.1,0.15,0.2,0.25,0.3,0.35,0.4,0.45,0.5,0.55,0.6,0.7,0.8,0.9,1.0,1.2,1.4,1.6,1.8,2.0,2.25,2.5,2.75,3.,3.5,4.,5.,6.,8.,10.};
  Double_t ptMin = 0., ptMax = 10.; 

  if(IsHptGenerator() == kTRUE) {
    nPtBins = 100;
    ptMin = 0.; ptMax = 100.; 
  }
  */

  Double_t yMin = -0.02, yMax = 0.02;
  Double_t zMin = -12.0, zMax = 12.0;
  if(GetAnalysisMode() == 3) { // TrackRef coordinate system
    yMin = -100.; yMax = 100.; 
    zMin = -100.; zMax = 100.; 
  }

  // res_y:res_z:res_phi,res_lambda:res_1pt:y:z:eta:phi:pt
  Int_t binsResolHisto[10]={100,100,100,100,100,25,50,30,144,nPtBins};
  Double_t minResolHisto[10]={-1.,-1.,-0.03,-0.03,-0.2, yMin, zMin,-1.5,0.,ptMin};
  Double_t maxResolHisto[10]={ 1., 1., 0.03, 0.03, 0.2, yMax, zMax, 1.5,2.*TMath::Pi(), ptMax};

  fResolHisto = new THnSparseF("fResolHisto","res_y:res_z:res_phi:res_lambda:res_1pt:y:z:eta:phi:pt",10,binsResolHisto,minResolHisto,maxResolHisto);
  fResolHisto->SetBinEdges(9,binsPt);

  fResolHisto->GetAxis(0)->SetTitle("y-y_{mc} (cm)");
  fResolHisto->GetAxis(1)->SetTitle("z-z_{mc} (cm)");
  fResolHisto->GetAxis(2)->SetTitle("#phi-#phi_{mc} (rad)");
  fResolHisto->GetAxis(3)->SetTitle("#lambda-#lambda_{mc} (rad)");
  fResolHisto->GetAxis(4)->SetTitle("(p_{Tmc}/p_{T}-1)");
  fResolHisto->GetAxis(5)->SetTitle("y_{mc} (cm)");
  fResolHisto->GetAxis(6)->SetTitle("z_{mc} (cm)");
  fResolHisto->GetAxis(7)->SetTitle("#eta_{mc}");
  fResolHisto->GetAxis(8)->SetTitle("#phi_{mc} (rad)");
  fResolHisto->GetAxis(9)->SetTitle("p_{Tmc} (GeV/c)");
  fResolHisto->Sumw2();

  //pull_y:pull_z:pull_lambda:pull_phi:pull_1pt:y:z:eta:phi:pt
  Int_t binsPullHisto[10]={100,100,100,100,100,50,50,30,144,nPtBins};
  Double_t minPullHisto[10]={-5.,-5.,-5.,-5.,-5.,yMin, zMin,-1.5, 0., ptMin};
  Double_t maxPullHisto[10]={ 5., 5., 5., 5., 5., yMax, zMax, 1.5, 2.*TMath::Pi(),ptMax};

  fPullHisto = new THnSparseF("fPullHisto","pull_y:pull_z:pull_phi:pull_lambda:pull_1pt:y:z:eta:phi:pt",10,binsPullHisto,minPullHisto,maxPullHisto);
  if(!IsHptGenerator()) fPullHisto->SetBinEdges(9,binsPt);

  fPullHisto->GetAxis(0)->SetTitle("(y-y_{mc})/#sigma");
  fPullHisto->GetAxis(1)->SetTitle("(z-z_{mc})/#sigma");
  fPullHisto->GetAxis(2)->SetTitle("(#phi-#phi_{mc})/#sigma");
  fPullHisto->GetAxis(3)->SetTitle("(#lambda-#lambda_{mc})/#sigma");
  fPullHisto->GetAxis(4)->SetTitle("(p_{Tmc}/p_{T}-1)/#sigma");
  fPullHisto->GetAxis(5)->SetTitle("y_{mc} (cm)");
  fPullHisto->GetAxis(6)->SetTitle("z_{mc} (cm)");
  fPullHisto->GetAxis(7)->SetTitle("#eta_{mc}");
  fPullHisto->GetAxis(8)->SetTitle("#phi_{mc} (rad)");
  fPullHisto->GetAxis(9)->SetTitle("p_{Tmc} (GeV/c)");
  fPullHisto->Sumw2();

  // Init cuts 
  if(!fCutsMC) 
    AliDebug(AliLog::kError, "ERROR: Cannot find AliMCInfoCuts object");
  if(!fCutsRC) 
    AliDebug(AliLog::kError, "ERROR: Cannot find AliRecInfoCuts object");

  // init folder
  fAnalysisFolder = CreateFolder("folderRes","Analysis Resolution Folder");
}

//_____________________________________________________________________________
void AliPerformanceRes::ProcessTPC(AliStack* const stack, AliESDtrack *const esdTrack)
{
  if(!esdTrack) return;

  // Fill TPC only resolution comparison information 
  const AliExternalTrackParam *track = esdTrack->GetTPCInnerParam();
  if(!track) return;

  Float_t dca[2], cov[3]; // dca_xy, dca_z, sigma_xy, sigma_xy_z, sigma_z
  esdTrack->GetImpactParametersTPC(dca,cov);
 
  //
  // Fill rec vs MC information
  //
  if(!stack) return;
  Int_t label = TMath::Abs(esdTrack->GetLabel()); 
  if(label == 0) return;

  TParticle* particle = stack->Particle(label);
  if(!particle) return;
  if(!particle->GetPDG()) return;
  if(particle->GetPDG()->Charge()==0) return;
  //printf("charge %d \n",particle->GetPDG()->Charge());

  Float_t delta1PtTPC, deltaYTPC, deltaZTPC, deltaPhiTPC, deltaLambdaTPC; 
  Float_t pull1PtTPC, pullYTPC, pullZTPC, pullPhiTPC, pullLambdaTPC; 

  Float_t mceta =  particle->Eta();
  Float_t mcphi =  particle->Phi();
  if(mcphi<0) mcphi += 2.*TMath::Pi();
  Float_t mcpt = particle->Pt();

  // nb. TPC clusters cut
  if (esdTrack->GetTPCNcls()<fCutsRC->GetMinNClustersTPC()) return;

  // select primaries
  if(TMath::Abs(dca[0])<fCutsRC->GetMaxDCAToVertexXY() && TMath::Abs(dca[1])<fCutsRC->GetMaxDCAToVertexZ()) 
  { 
    deltaYTPC= track->GetY()-particle->Vy();
    deltaZTPC = track->GetZ()-particle->Vz();
    deltaLambdaTPC = TMath::ATan2(track->Pz(),track->Pt())-TMath::ATan2(particle->Pz(),particle->Pt());
    deltaPhiTPC = TMath::ATan2(track->Py(),track->Px())-TMath::ATan2(particle->Py(),particle->Px());
    delta1PtTPC = (track->OneOverPt()-1./mcpt)*mcpt;

    pullYTPC= (track->GetY()-particle->Vy()) / TMath::Sqrt(track->GetSigmaY2());
    pullZTPC = (track->GetZ()-particle->Vz()) / TMath::Sqrt(track->GetSigmaZ2());
 
    Double_t sigma_lambda = 1./(1.+track->GetTgl()*track->GetTgl()) * TMath::Sqrt(track->GetSigmaTgl2()); 
    Double_t sigma_phi = 1./TMath::Sqrt(1-track->GetSnp()*track->GetSnp()) * TMath::Sqrt(track->GetSigmaSnp2());
    pullLambdaTPC = deltaLambdaTPC / sigma_lambda;
    pullPhiTPC = deltaPhiTPC / sigma_phi;

    //pullLambdaTPC = deltaLambdaTPC / TMath::Sqrt(track->GetSigmaTgl2());
    //pullPhiTPC = deltaPhiTPC / TMath::Sqrt(track->GetSigmaSnp2()); 
    if (mcpt) pull1PtTPC = (track->OneOverPt()-1./mcpt) / TMath::Sqrt(track->GetSigma1Pt2());
    else pull1PtTPC = 0.;

    Double_t vResolHisto[10] = {deltaYTPC,deltaZTPC,deltaPhiTPC,deltaLambdaTPC,delta1PtTPC,particle->Vy(),particle->Vz(),mceta,mcphi,mcpt};
    fResolHisto->Fill(vResolHisto);

    Double_t vPullHisto[10] = {pullYTPC,pullZTPC,pullPhiTPC,pullLambdaTPC,pull1PtTPC,particle->Vy(),particle->Vz(),mceta,mcphi,mcpt};
    fPullHisto->Fill(vPullHisto);
  }
}

//_____________________________________________________________________________
void AliPerformanceRes::ProcessTPCITS(AliStack* const stack, AliESDtrack *const esdTrack)
{
  // Fill resolution comparison information (TPC+ITS)
  if(!esdTrack) return;

  Float_t dca[2], cov[3]; // dca_xy, dca_z, sigma_xy, sigma_xy_z, sigma_z
  esdTrack->GetImpactParameters(dca,cov);
 
  //
  // Fill rec vs MC information
  //
  if(!stack) return;

  Int_t label = TMath::Abs(esdTrack->GetLabel()); 
  if(label == 0) return;

  TParticle* particle = stack->Particle(label);
  if(!particle) return;
  if(particle->GetPDG()->Charge()==0) return;
  //printf("charge %d \n",particle->GetPDG()->Charge());

  Float_t mceta =  particle->Eta();
  Float_t mcphi =  particle->Phi();
  if(mcphi<0) mcphi += 2.*TMath::Pi();
  Float_t mcpt = particle->Pt();

  if ((esdTrack->GetStatus()&AliESDtrack::kTPCrefit)==0) return; // TPC refit
  if (esdTrack->GetTPCNcls()<fCutsRC->GetMinNClustersTPC()) return; // min. nb. TPC clusters  
  Int_t clusterITS[200];
  if(esdTrack->GetITSclusters(clusterITS)<fCutsRC->GetMinNClustersITS()) return;  // min. nb. ITS clusters

  Float_t delta1PtTPC, deltaYTPC, deltaZTPC, deltaPhiTPC, deltaLambdaTPC; 
  Float_t pull1PtTPC, pullYTPC, pullZTPC, pullPhiTPC, pullLambdaTPC; 

  // select primaries
  if(TMath::Abs(dca[0])<fCutsRC->GetMaxDCAToVertexXY() && TMath::Abs(dca[1])<fCutsRC->GetMaxDCAToVertexZ()) 
  { 
    deltaYTPC= esdTrack->GetY()-particle->Vy();
    deltaZTPC = esdTrack->GetZ()-particle->Vz();
    deltaLambdaTPC = TMath::ATan2(esdTrack->Pz(),esdTrack->Pt())-TMath::ATan2(particle->Pz(),particle->Pt());
    deltaPhiTPC = TMath::ATan2(esdTrack->Py(),esdTrack->Px())-TMath::ATan2(particle->Py(),particle->Px());
    delta1PtTPC = (esdTrack->OneOverPt()-1./mcpt)*mcpt;

    pullYTPC= (esdTrack->GetY()-particle->Vy()) / TMath::Sqrt(esdTrack->GetSigmaY2());
    pullZTPC = (esdTrack->GetZ()-particle->Vz()) / TMath::Sqrt(esdTrack->GetSigmaZ2());

    Double_t sigma_lambda = 1./(1.+esdTrack->GetTgl()*esdTrack->GetTgl()) * TMath::Sqrt(esdTrack->GetSigmaTgl2()); 
    Double_t sigma_phi = 1./TMath::Sqrt(1-esdTrack->GetSnp()*esdTrack->GetSnp()) * TMath::Sqrt(esdTrack->GetSigmaSnp2());
    pullLambdaTPC = deltaLambdaTPC / sigma_lambda;
    pullPhiTPC = deltaPhiTPC / sigma_phi;

    //pullLambdaTPC = deltaLambdaTPC / TMath::Sqrt(esdTrack->GetSigmaTgl2());
    //pullPhiTPC = deltaPhiTPC / TMath::Sqrt(esdTrack->GetSigmaSnp2()); 
    if (mcpt) pull1PtTPC = (esdTrack->OneOverPt()-1./mcpt) / TMath::Sqrt(esdTrack->GetSigma1Pt2());
    else pull1PtTPC = 0.;

    Double_t vResolHisto[10] = {deltaYTPC,deltaZTPC,deltaPhiTPC,deltaLambdaTPC,delta1PtTPC,particle->Vy(),particle->Vz(),mceta,mcphi,mcpt};
    fResolHisto->Fill(vResolHisto);

    Double_t vPullHisto[10] = {pullYTPC,pullZTPC,pullPhiTPC,pullLambdaTPC,pull1PtTPC,particle->Vy(),particle->Vz(),mceta,mcphi,mcpt};
    fPullHisto->Fill(vPullHisto);
  }
}

//_____________________________________________________________________________
void AliPerformanceRes::ProcessConstrained(AliStack* const stack, AliESDtrack *const esdTrack)
{
  // Fill resolution comparison information (constarained parameters) 
  //
  if(!esdTrack) return;

  const AliExternalTrackParam * track = esdTrack->GetConstrainedParam();
  if(!track) return;

  Float_t dca[2], cov[3]; // dca_xy, dca_z, sigma_xy, sigma_xy_z, sigma_z
  esdTrack->GetImpactParameters(dca,cov);
 
  //
  // Fill rec vs MC information
  //
  if(!stack) return;

  Int_t label = TMath::Abs(esdTrack->GetLabel()); 
  if(label == 0) return;

  TParticle* particle = stack->Particle(label);
  if(!particle) return;
  if(particle->GetPDG()->Charge()==0) return;
  //printf("charge %d \n",particle->GetPDG()->Charge());

  Float_t mceta =  particle->Eta();
  Float_t mcphi =  particle->Phi();
  if(mcphi<0) mcphi += 2.*TMath::Pi();
  Float_t mcpt = particle->Pt();

  if ((esdTrack->GetStatus()&AliESDtrack::kTPCrefit)==0) return; // TPC refit
  if (esdTrack->GetTPCNcls()<fCutsRC->GetMinNClustersTPC()) return; // min. nb. TPC clusters

  Float_t delta1PtTPC, deltaYTPC, deltaZTPC, deltaPhiTPC, deltaLambdaTPC; 
  Float_t pull1PtTPC, pullYTPC, pullZTPC, pullPhiTPC, pullLambdaTPC; 

  // select primaries
  if(TMath::Abs(dca[0])<fCutsRC->GetMaxDCAToVertexXY() && TMath::Abs(dca[1])<fCutsRC->GetMaxDCAToVertexZ()) 
  { 
    deltaYTPC= track->GetY()-particle->Vy();
    deltaZTPC = track->GetZ()-particle->Vz();
    deltaLambdaTPC = TMath::ATan2(track->Pz(),track->Pt())-TMath::ATan2(particle->Pz(),particle->Pt());
    deltaPhiTPC = TMath::ATan2(track->Py(),track->Px())-TMath::ATan2(particle->Py(),particle->Px());
    delta1PtTPC = (track->OneOverPt()-1./mcpt)*mcpt;

    pullYTPC= (track->GetY()-particle->Vy()) / TMath::Sqrt(track->GetSigmaY2());
    pullZTPC = (track->GetZ()-particle->Vz()) / TMath::Sqrt(track->GetSigmaZ2());

    Double_t sigma_lambda = 1./(1.+track->GetTgl()*track->GetTgl()) * TMath::Sqrt(track->GetSigmaTgl2()); 
    Double_t sigma_phi = 1./TMath::Sqrt(1-track->GetSnp()*track->GetSnp()) * TMath::Sqrt(track->GetSigmaSnp2());
    pullLambdaTPC = deltaLambdaTPC / sigma_lambda;
    pullPhiTPC = deltaPhiTPC / sigma_phi;

    //pullLambdaTPC = deltaLambdaTPC / TMath::Sqrt(track->GetSigmaTgl2());
    //pullPhiTPC = deltaPhiTPC / TMath::Sqrt(track->GetSigmaSnp2()); 

    if (mcpt) pull1PtTPC = (track->OneOverPt()-1./mcpt) / TMath::Sqrt(track->GetSigma1Pt2());
    else pull1PtTPC = 0.;

    Double_t vResolHisto[10] = {deltaYTPC,deltaZTPC,deltaPhiTPC,deltaLambdaTPC,delta1PtTPC,particle->Vy(),particle->Vz(),mceta,mcphi,mcpt};
    fResolHisto->Fill(vResolHisto);

    Double_t vPullHisto[10] = {pullYTPC,pullZTPC,pullPhiTPC,pullLambdaTPC,pull1PtTPC,particle->Vy(),particle->Vz(),mceta,mcphi,mcpt};
    fPullHisto->Fill(vPullHisto);
  }
}
 
//_____________________________________________________________________________
void AliPerformanceRes::ProcessInnerTPC(AliMCEvent *const mcEvent, AliESDtrack *const esdTrack)
{
  //
  // Fill resolution comparison information (inner params at TPC reference point) 
  //
  if(!esdTrack) return;

  const AliExternalTrackParam * innerParam = esdTrack->GetInnerParam();
  if(!innerParam) return;

  // create new AliExternalTrackParam
  AliExternalTrackParam *track = new AliExternalTrackParam(*innerParam);
  if(!track) return;

  Float_t dca[2], cov[3]; // dca_xy, dca_z, sigma_xy, sigma_xy_z, sigma_z
  esdTrack->GetImpactParameters(dca,cov);
 
  //
  // Fill rec vs MC information
  //
  if(!mcEvent) return;

  Int_t label = TMath::Abs(esdTrack->GetLabel()); 
  if(label == 0) return;
  AliMCParticle *mcParticle = mcEvent->GetTrack(label);
  if(!mcParticle) return;

  // get the first TPC track reference
  AliTrackReference *ref0 = GetFirstTPCTrackRef(mcParticle);
  if(!ref0) return;

  // calculate alpha angle
  Double_t xyz[3] = {ref0->X(),ref0->Y(),ref0->Z()};
  Double_t alpha = TMath::ATan2(xyz[1],xyz[0]);
  //if (alpha<0) alpha += TMath::TwoPi();

  // rotate inner track to local coordinate system
  // and propagate to the radius of the first track referenco of TPC
  Double_t trRadius = TMath::Sqrt(xyz[1] * xyz[1] + xyz[0] * xyz[0]);
  Bool_t isOK = track->Propagate(alpha,trRadius,AliTracker::GetBz());
  if(!isOK) return;

  Float_t mceta =  -TMath::Log(TMath::Tan(0.5 * ref0->Theta()));
  Float_t mcphi =  ref0->Phi();
  if(mcphi<0) mcphi += 2.*TMath::Pi();
  Float_t mcpt = ref0->Pt();

  if ((esdTrack->GetStatus()&AliESDtrack::kTPCrefit)==0) return; // TPC refit
  if (esdTrack->GetTPCNcls()<fCutsRC->GetMinNClustersTPC()) return; // min. nb. TPC clusters

  Float_t delta1PtTPC, deltaYTPC, deltaZTPC, deltaPhiTPC, deltaLambdaTPC; 
  Float_t pull1PtTPC, pullYTPC, pullZTPC, pullPhiTPC, pullLambdaTPC; 

  // select primaries
  if(TMath::Abs(dca[0])<fCutsRC->GetMaxDCAToVertexXY() && TMath::Abs(dca[1])<fCutsRC->GetMaxDCAToVertexZ()) 
  { 
    //deltaYTPC= track->GetY()-ref0->Y();
    deltaYTPC= track->GetY(); // it corresponds to deltaY after alpha rotation 
    deltaZTPC = track->GetZ()-ref0->Z();
    deltaLambdaTPC = TMath::ATan2(track->Pz(),track->Pt())-TMath::ATan2(ref0->Pz(),ref0->Pt());
    deltaPhiTPC = TMath::ATan2(track->Py(),track->Px())-TMath::ATan2(ref0->Py(),ref0->Px());
    if(mcpt) delta1PtTPC = (track->OneOverPt()-1./mcpt)*mcpt;
    else delta1PtTPC = 0.;

    //pullYTPC= (track->GetY()-ref0->Y()) / TMath::Sqrt(track->GetSigmaY2());
    pullYTPC= track->GetY() / TMath::Sqrt(track->GetSigmaY2()); //it corresponds to deltaY after alpha rotation
    pullZTPC = (track->GetZ()-ref0->Z()) / TMath::Sqrt(track->GetSigmaZ2());

    //pullLambdaTPC = deltaLambdaTPC / TMath::Sqrt(track->GetSigmaTgl2());
    //pullPhiTPC = deltaPhiTPC / TMath::Sqrt(track->GetSigmaSnp2()); 

    Double_t sigma_lambda = 1./(1.+track->GetTgl()*track->GetTgl()) * TMath::Sqrt(track->GetSigmaTgl2()); 
    Double_t sigma_phi = 1./TMath::Sqrt(1-track->GetSnp()*track->GetSnp()) * TMath::Sqrt(track->GetSigmaSnp2());
    pullLambdaTPC = deltaLambdaTPC / sigma_lambda;
    pullPhiTPC = deltaPhiTPC / sigma_phi;

    if(mcpt) pull1PtTPC = (track->OneOverPt()-1./mcpt) / TMath::Sqrt(track->GetSigma1Pt2());
    else pull1PtTPC = 0.;

    Double_t vResolHisto[10] = {deltaYTPC,deltaZTPC,deltaPhiTPC,deltaLambdaTPC,delta1PtTPC,ref0->Y(),ref0->Z(),mceta,mcphi,mcpt};
    fResolHisto->Fill(vResolHisto);

    Double_t vPullHisto[10] = {pullYTPC,pullZTPC,pullPhiTPC,pullLambdaTPC,pull1PtTPC,ref0->Y(),ref0->Z(),mceta,mcphi,mcpt};
    fPullHisto->Fill(vPullHisto);
  }

  if(track) delete track;
}

//_____________________________________________________________________________
AliTrackReference * AliPerformanceRes::GetFirstTPCTrackRef(AliMCParticle *mcParticle) 
{
  // return first TPC track reference 
  if(!mcParticle) return 0;

  Int_t nTrackRef = mcParticle->GetNumberOfTrackReferences();
  AliTrackReference *ref0 = 0;
  for (Int_t iref = 0; iref < nTrackRef; iref++) { 
    ref0 = mcParticle->GetTrackReference(iref);
    if(ref0 && (ref0->DetectorId()==AliTrackReference::kTPC)) 
      break;
  }

return ref0;
}

//_____________________________________________________________________________
void AliPerformanceRes::Exec(AliMCEvent* const mcEvent, AliESDEvent* const esdEvent, const Bool_t bUseMC)
{
  // Process comparison information 
  //
  if(!esdEvent) 
  {
      AliDebug(AliLog::kError, "esdEvent not available");
      return;
  }
  AliHeader* header = 0;
  AliGenEventHeader* genHeader = 0;
  AliStack* stack = 0;
  TArrayF vtxMC(3);
  
  if(bUseMC)
  {
    if(!mcEvent) {
      AliDebug(AliLog::kError, "mcEvent not available");
      return;
    }

    // get MC event header
    header = mcEvent->Header();
    if (!header) {
      AliDebug(AliLog::kError, "Header not available");
      return;
    }
    // MC particle stack
    stack = mcEvent->Stack();
    if (!stack) {
      AliDebug(AliLog::kError, "Stack not available");
      return;
    }

    // get MC vertex
    genHeader = header->GenEventHeader();
    if (!genHeader) {
      AliDebug(AliLog::kError, "Could not retrieve genHeader from Header");
      return;
    }
    genHeader->PrimaryVertex(vtxMC);

  } // end bUseMC

  //  Process events
  for (Int_t iTrack = 0; iTrack < esdEvent->GetNumberOfTracks(); iTrack++) 
  { 
    AliESDtrack*track = esdEvent->GetTrack(iTrack);
    if(!track) continue;

    if(GetAnalysisMode() == 0) ProcessTPC(stack,track);
    else if(GetAnalysisMode() == 1) ProcessTPCITS(stack,track);
    else if(GetAnalysisMode() == 2) ProcessConstrained(stack,track);
    else if(GetAnalysisMode() == 3) ProcessInnerTPC(mcEvent,track);
    else {
      printf("ERROR: AnalysisMode %d \n",fAnalysisMode);
      return;
    }
  }
}

//_____________________________________________________________________________
TH1F* AliPerformanceRes::MakeResol(TH2F * his, Int_t integ, Bool_t type, Int_t cut){
  // Create resolution histograms
 
  TH1F *hisr, *hism;
  if (!gPad) new TCanvas;
  hisr = AliTreeDraw::CreateResHistoII(his,&hism,integ,kTRUE,cut);
  if (type) return hism;
  else 
    return hisr;
}

//_____________________________________________________________________________
void AliPerformanceRes::Analyse(){
  // Analyse comparison information and store output histograms
  // in the folder "folderRes"
  //
  TH1::AddDirectory(kFALSE);
  TH1F *h=0;
  TH2F *h2D=0;
  TObjArray *aFolderObj = new TObjArray;

  // write results in the folder 
  TCanvas * c = new TCanvas("Phi resol Tan","Phi resol Tan");
  c->cd();

  char name[256];
  char title[256];

  for(Int_t i=0; i<5; i++) 
  {
    for(Int_t j=5; j<10; j++) 
    {
      if(j!=7) fResolHisto->GetAxis(7)->SetRangeUser(-0.9,0.9); // eta window
      fResolHisto->GetAxis(9)->SetRangeUser(0.16,10.); // pt threshold
      if(GetAnalysisMode() == 3) fResolHisto->GetAxis(5)->SetRangeUser(-80.,80.); // y range

      h2D = (TH2F*)fResolHisto->Projection(i,j);

      h = AliPerformanceRes::MakeResol(h2D,1,0,100);
      sprintf(name,"h_res_%d_vs_%d",i,j);

      /*
      if(i<2) {
        h->SetMinimum(0.);
        h->SetMaximum(1.);
      } else {
        h->SetMinimum(0.);
        h->SetMaximum(0.2);
      }
      */
      h->SetName(name);

      h->GetXaxis()->SetTitle(fResolHisto->GetAxis(j)->GetTitle());
      sprintf(title,"%s %s",fResolHisto->GetAxis(i)->GetTitle(),"(resolution)");
      h->GetYaxis()->SetTitle(title);
      sprintf(title,"%s vs %s",title,fResolHisto->GetAxis(j)->GetTitle());
      h->SetTitle(title);

      if(j==9) h->SetBit(TH1::kLogX);    
      aFolderObj->Add(h);

      h = AliPerformanceRes::MakeResol(h2D,1,1,100);
      //h = (TH1F*)arr->At(1);
      sprintf(name,"h_mean_res_%d_vs_%d",i,j);
      /*
      h->SetMinimum(-0.05);
      h->SetMaximum(0.05);
      if(i<2) {
        h->SetMinimum(-0.05);
        h->SetMaximum(0.05);
      } else {
        h->SetMinimum(-0.02);
        h->SetMaximum(0.02);
      }
      */
      h->SetName(name);

      h->GetXaxis()->SetTitle(fResolHisto->GetAxis(j)->GetTitle());
      sprintf(title,"%s %s",fResolHisto->GetAxis(i)->GetTitle(),"(mean)");
      h->GetYaxis()->SetTitle(title);

      sprintf(title,"%s vs %s",title,fResolHisto->GetAxis(j)->GetTitle());
      h->SetTitle(title);

      if(j==9) h->SetBit(TH1::kLogX);    
      aFolderObj->Add(h);

      fResolHisto->GetAxis(7)->SetRangeUser(-1.5,1.5);
      fResolHisto->GetAxis(9)->SetRangeUser(0.0,10.);

      //
      if(j!=7) fPullHisto->GetAxis(7)->SetRangeUser(-0.9,0.9); // eta window
      fPullHisto->GetAxis(9)->SetRangeUser(0.16,10.);  // pt threshold
      if(GetAnalysisMode() == 3) fPullHisto->GetAxis(5)->SetRangeUser(-80.,80.); // y range

      h2D = (TH2F*)fPullHisto->Projection(i,j);

      h = AliPerformanceRes::MakeResol(h2D,1,0,100);
      sprintf(name,"h_pull_%d_vs_%d",i,j);
      //h->SetMinimum(0.);
      //h->SetMaximum(2.);
      h->SetName(name);

      h->GetXaxis()->SetTitle(fPullHisto->GetAxis(j)->GetTitle());
      sprintf(title,"%s %s",fPullHisto->GetAxis(i)->GetTitle(),"(resolution)");
      h->GetYaxis()->SetTitle(title);
      sprintf(title,"%s vs %s",title,fPullHisto->GetAxis(j)->GetTitle());
      h->SetTitle(title);

      if(j==9) h->SetBit(TH1::kLogX);    
      aFolderObj->Add(h);

      h = AliPerformanceRes::MakeResol(h2D,1,1,100);
      sprintf(name,"h_mean_pull_%d_vs_%d",i,j);
      //h->SetMinimum(-0.2);
      //h->SetMaximum(0.2);
      h->SetName(name);

      h->GetXaxis()->SetTitle(fPullHisto->GetAxis(j)->GetTitle());
      sprintf(title,"%s %s",fPullHisto->GetAxis(i)->GetTitle(),"(mean)");
      h->GetYaxis()->SetTitle(title);
      sprintf(title,"%s vs %s",title,fPullHisto->GetAxis(j)->GetTitle());
      h->SetTitle(title);

      if(j==9) h->SetBit(TH1::kLogX);    
      aFolderObj->Add(h);

      fPullHisto->GetAxis(7)->SetRangeUser(-1.5,1.5);
      fPullHisto->GetAxis(9)->SetRangeUser(0.0,10.);
    }
  }

  // export objects to analysis folder
  fAnalysisFolder = ExportToFolder(aFolderObj);

  // delete only TObjArray
  if(aFolderObj) delete aFolderObj;
}

//_____________________________________________________________________________
TFolder* AliPerformanceRes::ExportToFolder(TObjArray * array) 
{
  // recreate folder avery time and export objects to new one
  //
  AliPerformanceRes * comp=this;
  TFolder *folder = comp->GetAnalysisFolder();

  TString name, title;
  TFolder *newFolder = 0;
  Int_t i = 0;
  Int_t size = array->GetSize();

  if(folder) { 
     // get name and title from old folder
     name = folder->GetName();  
     title = folder->GetTitle();  

	 // delete old one
     delete folder;

	 // create new one
     newFolder = CreateFolder(name.Data(),title.Data());
     newFolder->SetOwner();

	 // add objects to folder
     while(i < size) {
	   newFolder->Add(array->At(i));
	   i++;
	 }
  }

return newFolder;
}

//_____________________________________________________________________________
Long64_t AliPerformanceRes::Merge(TCollection* const list) 
{
  // Merge list of objects (needed by PROOF)

  if (!list)
  return 0;

  if (list->IsEmpty())
  return 1;

  TIterator* iter = list->MakeIterator();
  TObject* obj = 0;

  // collection of generated histograms
  Int_t count=0;
  while((obj = iter->Next()) != 0) 
  {
  AliPerformanceRes* entry = dynamic_cast<AliPerformanceRes*>(obj);
  if (entry == 0) continue; 

  fResolHisto->Add(entry->fResolHisto);
  fPullHisto->Add(entry->fPullHisto);

  count++;
  }

return count;
}

//_____________________________________________________________________________
TFolder* AliPerformanceRes::CreateFolder(TString name,TString title) { 
// create folder for analysed histograms
//
TFolder *folder = 0;
  folder = new TFolder(name.Data(),title.Data());

  return folder;
}
