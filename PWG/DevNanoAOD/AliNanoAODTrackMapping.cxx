#include "AliNanoAODTrackMapping.h"
#include "TString.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "AliLog.h"
#include <iostream>
#include "AliAnalysisManager.h"
#include "AliVEventHandler.h"


ClassImp(AliNanoAODTrackMapping)

AliNanoAODTrackMapping * AliNanoAODTrackMapping::fInstance = 0;
TString  AliNanoAODTrackMapping::fMappingString = "";

AliNanoAODTrackMapping::AliNanoAODTrackMapping() :
  TObject(),
  fPt(-1),      	  
  fPhi(-1),		  
  fTheta(-1),		  
  fChi2PerNDF(-1),	  
  fPosX(-1),		  
  fPosY(-1),		  
  fPosZ(-1),		  
  fPDCAX(-1),		  
  fPDCAY(-1),		  
  fPDCAZ(-1),		  
  fPosDCAx(-1),	  
  fPosDCAy(-1),	  
  fRAtAbsorberEnd(-1),  
  fTPCncls(-1),	  
  fID(-1),		  
  fTPCnclsF(-1),	  
  fTPCNCrossedRows(-1), 
  fTrackPhiOnEMCal(-1), 
  fTrackEtaOnEMCal(-1), 
  fITSsignal(-1),	  
  fTPCsignal(-1),	  
  fTPCsignalTuned(-1),  
  fTPCsignalN(-1),	  
  fTPCmomentum(-1),	  
  fTPCTgl(-1),	  
  fTOFsignal(-1),	  
  fintegratedLength(-1),
  fTOFsignalTuned(-1),  
  fHMPIDsignal(-1),	  
  fHMPIDoccupancy(-1),  
  fTRDsignal(-1),	  
  fTRDChi2(-1),	  
  fTRDnSlices(-1),	  
  fIsMuonTrack(-1),
  fTPCnclsS(-1),
  fFilterMap(-1),
  fTOFBunchCrossing(-1),
  fcovmat{-1},
  fTOFchi2{-1},
  fTOFsignalDz{-1},
  fTOFsignalDx{-1}
{ 
  /// default ctor

  for(Int_t i=0; i<21;i++){
  fcovmat[i]=-1;
  }
}

AliNanoAODTrackMapping::AliNanoAODTrackMapping(const char * mappingString) :
  TObject(),
  fPt(-1),      	  
  fPhi(-1),		  
  fTheta(-1),		  
  fChi2PerNDF(-1),	  
  fPosX(-1),		  
  fPosY(-1),		  
  fPosZ(-1),		  
  fPDCAX(-1),		  
  fPDCAY(-1),		  
  fPDCAZ(-1),		  
  fPosDCAx(-1),	  
  fPosDCAy(-1),	  
  fRAtAbsorberEnd(-1),  
  fTPCncls(-1),	  
  fID(-1),		  
  fTPCnclsF(-1),	  
  fTPCNCrossedRows(-1), 
  fTrackPhiOnEMCal(-1), 
  fTrackEtaOnEMCal(-1), 
  fITSsignal(-1),	  
  fTPCsignal(-1),	  
  fTPCsignalTuned(-1),  
  fTPCsignalN(-1),	  
  fTPCmomentum(-1),	  
  fTPCTgl(-1),	  
  fTOFsignal(-1),	  
  fintegratedLength(-1),
  fTOFsignalTuned(-1),  
  fHMPIDsignal(-1),	  
  fHMPIDoccupancy(-1),  
  fTRDsignal(-1),	  
  fTRDChi2(-1),	  
  fTRDnSlices(-1),	  
  fIsMuonTrack(-1),
  fTPCnclsS(-1),
  fFilterMap(-1),
  fTOFBunchCrossing(-1),
  fcovmat{-1},
  fTOFchi2{-1},
  fTOFsignalDz{-1},
  fTOFsignalDx{-1}
{
  /// ctor

  for(Int_t i=0; i<21;i++){
  fcovmat[i]=-1;
  }
  //std::cout << "Standard construct " << mappingString << std::endl;
  
  if (fInstance) {
    AliWarning("Cannot instantiate this class twice");
    if (fMappingString != mappingString) AliFatal ("Attempt to initialize with a differnt mapping string");
    return ;
  }
  fMappingString = mappingString;

  // Tokenize list of variables
  TString varString(mappingString);
  TObjArray * vars = varString.Tokenize(",");
  // Allocate the internal array, based on size of the tokens array
  fSize = vars->GetSize();
  // Loop over all tokens, check sanity of the vars,
  TIter it(vars);
  TObjString *token  = 0;
  Int_t index=0;
  while ((token = (TObjString*) it.Next())) {
    TString var = token->GetString().Strip(TString::kBoth, ' '); // remove trailing and leading spaces        

    if     (var == "pt"               ) fPt                = index++;
    else if(var == "phi"              ) fPhi               = index++;
    else if(var == "theta"            ) fTheta             = index++; // FIXME: consider adding a "eta" variable explicitly (possibly with a check for theta aldready there), so that you don't have to carry over also "theta" in case you only need eta.
    else if(var == "chi2perNDF"       ) fChi2PerNDF        = index++;
    else if(var == "posx"             ) fPosX              = index++;
    else if(var == "posy"             ) fPosY              = index++;
    else if(var == "posz"             ) fPosZ              = index++;
    else if(var == "pDCAx"            ) fPDCAX             = index++;
    else if(var == "pDCAy"            ) fPDCAY             = index++;
    else if(var == "pDCAz"            ) fPDCAZ             = index++;
    else if(var == "posDCAx"          ) fPosDCAx           = index++;
    else if(var == "posDCAy"          ) fPosDCAy           = index++;
    else if(var == "RAtAbsorberEnd"   ) fRAtAbsorberEnd    = index++;
    else if(var == "TPCncls"          ) fTPCncls           = index++;
    else if(var == "ID"               ) fID                = index++;
    else if(var == "TPCnclsF"         ) fTPCnclsF          = index++;
    else if(var == "TPCNCrossedRows"  ) fTPCNCrossedRows   = index++;
    else if(var == "TrackPhiOnEMCal"  ) fTrackPhiOnEMCal   = index++;
    else if(var == "TrackEtaOnEMCal"  ) fTrackEtaOnEMCal   = index++;
    else if(var == "TrackPtOnEMCal"   ) fTrackPtOnEMCal    = index++;
    else if(var == "ITSsignal"        ) fITSsignal         = index++;
    else if(var == "TPCsignal"        ) fTPCsignal         = index++;
    else if(var == "TPCsignalTuned"   ) fTPCsignalTuned    = index++;
    else if(var == "TPCsignalN"       ) fTPCsignalN        = index++;
    else if(var == "TPCmomentum"      ) fTPCmomentum       = index++;
    else if(var == "TPCTgl"           ) fTPCTgl            = index++;
    else if(var == "TOFsignal"        ) fTOFsignal         = index++;
    else if(var == "integratedLength" ) fintegratedLength  = index++;
    else if(var == "TOFsignalTuned"   ) fTOFsignalTuned    = index++;
    else if(var == "HMPIDsignal"      ) fHMPIDsignal       = index++;
    else if(var == "HMPIDoccupancy"   ) fHMPIDoccupancy    = index++;
    else if(var == "TRDsignal"        ) fTRDsignal         = index++;
    else if(var == "TRDChi2"          ) fTRDChi2           = index++;
    else if(var == "TRDnSlices"       ) fTRDnSlices        = index++;
    else if(var == "IsMuonTrack"      ) fIsMuonTrack       = index++;
    else if(var == "TPCnclsS"         ) fTPCnclsS          = index++;
    else if(var == "FilterMap"        ) fFilterMap         = index++;
    else if(var == "TOFBunchCrossing" ) fTOFBunchCrossing  = index++;
    else if(var == "covmat"           ) {
        for(Int_t i=0;i<21;i++){
            fcovmat[i] = index++;
        }
    }
    else if(var == "TOFchi2"          ) fTOFchi2           = index++;
    else if(var == "TOFsignalDz"      ) fTOFsignalDz       = index++;
    else if(var == "TOFsignalDx"      ) fTOFsignalDx       = index++;

    else if (var.BeginsWith("cst") || var.BeginsWith("PID.")) {
      Info("AliNanoAODTrackMapping::AliNanoAODTrackMapping", "ADDING %i %i %s",index,fMapCstVar[var],var.Data());
      fMapCstVar[var] = index++;
    } else
       AliFatal(Form("Invalid var [%s]", var.Data()));

  }
  fSize = index;
  if(vars) vars->Delete();
 

}

Int_t AliNanoAODTrackMapping::GetVarIndex(TString varName){

    if     (varName == "pt"               ) return fPt               ;
    else if(varName == "phi"              ) return fPhi              ;
    else if(varName == "theta"            ) return fTheta            ; 
    else if(varName == "chi2perNDF"       ) return fChi2PerNDF       ;
    else if(varName == "posx"             ) return fPosX             ;
    else if(varName == "posy"             ) return fPosY             ;
    else if(varName == "posz"             ) return fPosZ             ;
    else if(varName == "pDCAx"            ) return fPDCAX            ;
    else if(varName == "pDCAy"            ) return fPDCAY            ;
    else if(varName == "pDCAz"            ) return fPDCAZ            ;
    else if(varName == "posDCAx"          ) return fPosDCAx          ;
    else if(varName == "posDCAy"          ) return fPosDCAy          ;
    else if(varName == "RAtAbsorberEnd"   ) return fRAtAbsorberEnd   ;
    else if(varName == "TPCncls"          ) return fTPCncls          ;
    else if(varName == "ID"               ) return fID               ;
    else if(varName == "TPCnclsF"         ) return fTPCnclsF         ;
    else if(varName == "TPCNCrossedRows"  ) return fTPCNCrossedRows  ;
    else if(varName == "TrackPhiOnEMCal"  ) return fTrackPhiOnEMCal  ;
    else if(varName == "TrackEtaOnEMCal"  ) return fTrackEtaOnEMCal  ;
    else if(varName == "TrackPtOnEMCal"   ) return fTrackPtOnEMCal   ;
    else if(varName == "ITSsignal"        ) return fITSsignal        ;
    else if(varName == "TPCsignal"        ) return fTPCsignal        ;
    else if(varName == "TPCsignalTuned"   ) return fTPCsignalTuned   ;
    else if(varName == "TPCsignalN"       ) return fTPCsignalN       ;
    else if(varName == "TPCmomentum"      ) return fTPCmomentum      ;
    else if(varName == "TPCTgl"           ) return fTPCTgl           ;
    else if(varName == "TOFsignal"        ) return fTOFsignal        ;
    else if(varName == "integratedLength" ) return fintegratedLength ;
    else if(varName == "TOFsignalTuned"   ) return fTOFsignalTuned   ;
    else if(varName == "HMPIDsignal"      ) return fHMPIDsignal      ;
    else if(varName == "HMPIDoccupancy"   ) return fHMPIDoccupancy   ;
    else if(varName == "TRDsignal"        ) return fTRDsignal        ;
    else if(varName == "TRDChi2"          ) return fTRDChi2          ;
    else if(varName == "TRDnSlices"       ) return fTRDnSlices       ;
    else if(varName == "IsMuonTrack"      ) return fIsMuonTrack      ;
    else if(varName == "TPCnclsS"         ) return fTPCnclsS         ;
    else if(varName == "FilterMap"        ) return fFilterMap        ;
    else if(varName == "TOFBunchCrossing" ) return fTOFBunchCrossing ;
    else if(varName == "covmat0"          ) return fcovmat[0]        ;
    else if(varName == "TOFchi2"          ) return fTOFchi2          ;
    else if(varName == "TOFsignalDz"      ) return fTOFsignalDz      ;
    else if(varName == "TOFsignalDx"      ) return fTOFsignalDx      ;

    std::map<TString,Int_t>::iterator it = fMapCstVar.find(varName); // FIXME: do I need to delete "it"?
    if(it != fMapCstVar.end()) {
      //element found;
	return it->second;
    }

    return -1;
  
}

const char * AliNanoAODTrackMapping::GetVarName(Int_t index) const {
  /// Get Variable name from index

    if     (index == fPt               )  return "pt"               ;
    else if(index == fPhi              )  return "phi"              ;
    else if(index == fTheta            )  return "theta"            ;
    else if(index == fChi2PerNDF       )  return "chi2perNDF"       ;
    else if(index == fPosX             )  return "posx"             ;
    else if(index == fPosY             )  return "posy"             ;
    else if(index == fPosZ             )  return "posz"             ;
    else if(index == fPDCAX            )  return "pDCAx"            ;
    else if(index == fPDCAY            )  return "pDCAy"            ;
    else if(index == fPDCAZ            )  return "pDCAz"            ;
    else if(index == fPosDCAx          )  return "posDCAx"          ;
    else if(index == fPosDCAy          )  return "posDCAy"          ;
    else if(index == fRAtAbsorberEnd   )  return "RAtAbsorberEnd"   ;
    else if(index == fTPCncls          )  return "TPCncls"          ;
    else if(index == fID               )  return "ID"               ;
    else if(index == fTPCnclsF         )  return "TPCnclsF"         ;
    else if(index == fTPCNCrossedRows  )  return "TPCNCrossedRows"  ;
    else if(index == fTrackPhiOnEMCal  )  return "TrackPhiOnEMCal"  ;
    else if(index == fTrackEtaOnEMCal  )  return "TrackEtaOnEMCal"  ;
    else if(index == fTrackPtOnEMCal   )  return "TrackPtOnEMCal"   ;
    else if(index == fITSsignal        )  return "ITSsignal"        ;
    else if(index == fTPCsignal        )  return "TPCsignal"        ;
    else if(index == fTPCsignalTuned   )  return "TPCsignalTuned"   ;
    else if(index == fTPCsignalN       )  return "TPCsignalN"       ;
    else if(index == fTPCmomentum      )  return "TPCmomentum"      ;
    else if(index == fTPCTgl           )  return "TPCTgl"           ;
    else if(index == fTOFsignal        )  return "TOFsignal"        ;
    else if(index == fintegratedLength )  return "integratedLength" ;
    else if(index == fTOFsignalTuned   )  return "TOFsignalTuned"   ;
    else if(index == fHMPIDsignal      )  return "HMPIDsignal"      ;
    else if(index == fHMPIDoccupancy   )  return "HMPIDoccupancy"   ;
    else if(index == fTRDsignal        )  return "TRDsignal"        ;
    else if(index == fTRDChi2          )  return "TRDChi2"          ;
    else if(index == fTRDnSlices       )  return "TRDnSlices"       ;
    else if(index == fIsMuonTrack      )  return "IsMuonTrack"      ;
    else if(index == fTPCnclsS         )  return "TPCnclsS"         ;
    else if(index == fFilterMap        )  return "FilterMap"        ;
    else if(index == fTOFBunchCrossing )  return "TOFBunchCrossing" ;
    else if(index == fTOFchi2          )  return "TOFchi2"          ;
    else if(index == fTOFsignalDz      )  return "TOFsignalDz"      ;
    else if(index == fTOFsignalDx      )  return "TOFsignalDx"      ;
    
    for (Int_t i=0; i<21; i++){
        
        if(index == fcovmat[i]) return TString::Format("covmat%d",i).Data();
        
    }
    if (index >= fSize) {
      AliWarning(Form("Invalid index %d", index));
      return "<Invalid index>";
    }
    else {
      std::map<TString,Int_t>::const_iterator it ;
      for (it = fMapCstVar.begin(); it != fMapCstVar.end(); ++it) {
	      if(it->second == index) return it->first.Data();
      }      
    }
    return "<YOU SHOULD NEVER GET THIS>";// Should never happen
}

void  AliNanoAODTrackMapping::Print(const Option_t* /*opt*/) const {
  std::cout << "Printing AliNanoAODTrackMapping" << std::endl;
  
  for (Int_t ivar = 0; ivar<fSize; ivar++) {
    std::cout << " " << ivar << " " << GetVarName(ivar) << std::endl;
  }

}


void  AliNanoAODTrackMapping::LoadInstance() 
{
  if(!fInstance) { // try to get it from the current file
    Printf("AliNanoAODTrackMapping::LoadInstance() instance from file");
    AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
    if (!mgr)
      AliFatalClass("No AliAnalysisManager");
    
    TList* userInfo = mgr->GetInputEventHandler()->GetUserInfo();
    if (!userInfo)
      AliFatalClass("Cannot retrieve user info");
    
    userInfo->Print();
    fInstance = new AliNanoAODTrackMapping(*((AliNanoAODTrackMapping*) userInfo->FindObject("AliNanoAODTrackMapping")));
    std::cout << " from tree: " << fInstance << std::endl;
  }
  // Otherwise try to get it from the current directory
  if (!fInstance) 
    fInstance = (AliNanoAODTrackMapping *) gDirectory->Get("AliNanoAODTrackMapping");
  if(!fInstance)
    AliFatalClass("AliNanoAODTrackMapping::LoadInstance(): Not initialized and cannot find in current file");
}