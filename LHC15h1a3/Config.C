// One can use the configuration macro in compiled mode by
// root [0] gSystem->Load("libgeant321");
// root [0] gSystem->SetIncludePath("-I$ROOTSYS/include -I$ALICE_ROOT/include\
//                   -I$ALICE_ROOT -I$ALICE/geant3/TGeant3");
// root [0] .x grun.C(1,"ConfigPPR.C++")

#if !defined(__CINT__) || defined(__MAKECINT__)
#include <Riostream.h>
#include <TRandom.h>
#include <TSystem.h>
#include <TVirtualMC.h>
#include <TGeant3TGeo.h>
#include <TPDGCode.h>
#include <TF1.h>
#include "STEER/AliRunLoader.h"
#include "STEER/AliRun.h"
#include "STEER/AliConfig.h"
#include "STEER/AliGenerator.h"
#include "STEER/AliLog.h"
#include "PYTHIA6/AliDecayerPythia.h"
#include "EVGEN/AliGenHIJINGpara.h"
#include "THijing/AliGenHijing.h"
#include "EVGEN/AliGenCocktail.h"
#include "EVGEN/AliGenSlowNucleons.h"
#include "EVGEN/AliSlowNucleonModelExp.h"
#include "EVGEN/AliGenParam.h"
#include "EVGEN/AliGenMUONlib.h"
#include "EVGEN/AliGenSTRANGElib.h"
#include "EVGEN/AliGenMUONCocktail.h"
#include "EVGEN/AliGenCocktail.h"
#include "EVGEN/AliGenGeVSim.h"
#include "EVGEN/AliGeVSimParticle.h"
#include "PYTHIA6/AliGenPythia.h"
#include "TDPMjet/AliGenDPMjet.h"
#include "STEER/AliMagF.h"
#include "STRUCT/AliBODY.h"
#include "STRUCT/AliMAG.h"
#include "STRUCT/AliABSOv3.h"
#include "STRUCT/AliDIPOv3.h"
#include "STRUCT/AliHALLv3.h"
#include "STRUCT/AliFRAMEv2.h"
#include "STRUCT/AliSHILv3.h"
#include "STRUCT/AliPIPEv3.h"
#include "ITS/AliITSv11.h"
#include "TPC/AliTPCv2.h"
#include "TOF/AliTOFv6T0.h"
#include "HMPID/AliHMPIDv3.h"
#include "ZDC/AliZDCv4.h"
#include "TRD/AliTRDv1.h"
#include "TRD/AliTRDgeometry.h"
#include "FMD/AliFMDv1.h"
#include "MUON/AliMUONv1.h"
#include "PHOS/AliPHOSv1.h"
#include "PMD/AliPMDv1.h"
#include "T0/AliT0v1.h"
#include "EMCAL/AliEMCALv2.h"
#include "ACORDE/AliACORDEv1.h"
#include "VZERO/AliVZEROv7.h"
#endif

enum PprRun_t 
{
   kPythia6, kPhojet, kPythia8, kPythia8PlusSignals, kRunMax
};

const char* pprRunName[] = {
	"kPythia6", "kPhojet", "kPythia8", "kPythia8PlusSignals"
};

enum PprRad_t
{
    kGluonRadiation, kNoGluonRadiation
};

enum PprTrigConf_t
{
    kDefaultPPTrig, kDefaultPbPbTrig
};

const char * pprTrigConfName[] = {
    "p-p","Pb-Pb"
};

// This part for configuration    

static PprRun_t srun = kPythia8;
static PprRad_t srad = kGluonRadiation;
static AliMagF::BMap_t smag = AliMagF::k5kG;
static Int_t    sseed = 0; //Set 0 to use the current time
static PprTrigConf_t strig = kDefaultPPTrig; // default pp trigger configuration
static Float_t energy = 8000.;
// Comment line 
static TString  comment;
TDatime dt;
static UInt_t seed    = dt.Get();
static Int_t runNumber = 0;
// Functions
Float_t EtaToTheta(Float_t arg);
AliGenerator* GeneratorFactory(PprRun_t srun);
void ProcessEnvironmentVars();

void Config()
{
    // ThetaRange is (0., 180.). It was (0.28,179.72) 7/12/00 09:00
    // Theta range given through pseudorapidity limits 22/6/2001

    // Get settings from environment variables
    ProcessEnvironmentVars();

    // Set Random Number seed
    gRandom->SetSeed(sseed);
    cout<<"Seed for random number generation= "<<gRandom->GetSeed()<<endl; 


   // libraries required by geant321
#if defined(__CINT__)
    gSystem->Load("liblhapdf");
    gSystem->Load("libEGPythia6");
    gSystem->Load("libpythia6");
    gSystem->Load("libAliPythia6");
    gSystem->Load("libgeant321");
	 gSystem->Load("libpythia8.so");
    gSystem->Load("libAliPythia8.so");
    gSystem->Setenv("PYTHIA8DATA", gSystem->ExpandPathName("$ALICE_ROOT/PYTHIA8/pythia8205/share/Pythia8/xmldoc"));
    gSystem->Setenv("LHAPDF",      gSystem->ExpandPathName("$ALICE_ROOT/LHAPDF"));
    gSystem->Setenv("LHAPATH",     gSystem->ExpandPathName("$ALICE_ROOT/LHAPDF/PDFsets"));
#endif

	new     TGeant3TGeo("C++ Interface to Geant3");

	// Output every 100 tracks
	((TGeant3*)gMC)->SetSWIT(4,100);

	AliRunLoader* rl=0x0;
	AliLog::Message(AliLog::kInfo, "Creating Run Loader", "", "", "Config()"," ConfigPPR.C", __LINE__);

	rl = AliRunLoader::Open("galice.root",
				AliConfig::GetDefaultEventFolderName(),
				"recreate");
	if (rl == 0x0){
		gAlice->Fatal("Config.C","Can not instatiate the Run Loader");
		return;
	}
	rl->SetCompressionLevel(2);
	rl->SetNumberOfEventsPerFile(10000);
	gAlice->SetRunLoader(rl);

	// Set the trigger configuration
	AliSimulation::Instance()->SetTriggerConfig(pprTrigConfName[strig]);
	cout<<"Trigger configuration is set to  "<<pprTrigConfName[strig]<<endl;

	//
	// Set External decayer
	AliDecayer *decayer = new AliDecayerPythia();
	decayer->SetForceDecay(kAll);
	decayer->Init();
	gMC->SetExternalDecayer(decayer);
	//
	//
	//=======================================================================
	//
	//=======================================================================
	// ************* STEERING parameters FOR ALICE SIMULATION **************
	// --- Specify event type to be tracked through the ALICE setup
	// --- All positions are in cm, angles in degrees, and P and E in GeV

	gMC->SetProcess("DCAY",1);
	gMC->SetProcess("PAIR",1);
	gMC->SetProcess("COMP",1);
	gMC->SetProcess("PHOT",1);
	gMC->SetProcess("PFIS",0);
	gMC->SetProcess("DRAY",0);
	gMC->SetProcess("ANNI",1);
	gMC->SetProcess("BREM",1);
	gMC->SetProcess("MUNU",1);
	gMC->SetProcess("CKOV",1);
	gMC->SetProcess("HADR",1);
	gMC->SetProcess("LOSS",2);
	gMC->SetProcess("MULS",1);
	gMC->SetProcess("RAYL",1);

	Float_t cut = 1.e-3;        // 1MeV cut by default
	Float_t tofmax = 1.e10;

	gMC->SetCut("CUTGAM", cut);
	gMC->SetCut("CUTELE", cut);
	gMC->SetCut("CUTNEU", cut);
	gMC->SetCut("CUTHAD", cut);
	gMC->SetCut("CUTMUO", cut);
	gMC->SetCut("BCUTE",  cut); 
	gMC->SetCut("BCUTM",  cut); 
	gMC->SetCut("DCUTE",  cut); 
	gMC->SetCut("DCUTM",  cut); 
	gMC->SetCut("PPCUTM", cut);
	gMC->SetCut("TOFMAX", tofmax); 

	// Generator Configuration
	AliGenerator* gener = GeneratorFactory(srun);
	gener->SetOrigin(0, 0, 0);    // vertex position
	gener->SetSigma(0, 0, 5.3);   // Sigma in (X,Y,Z) (cm) on IP position
	// Removed as per https://savannah.cern.ch/task/?33183#comment61
	// gener->SetCutVertexZ(1.);     // Truncate at 1 sigma
	gener->SetVertexSmear(kPerEvent);
	gener->SetTrackingFlag(1);
	gener->Init();

	switch (srun){
		case kPythia8:
		{
			AliPythia8::Instance()->ReadString("111:mayDecay  =  on");
		}
		break;
		case kPythia8PlusSignals:
		{
			AliPythia8::Instance()->ReadString("111:mayDecay  =  on");
		}
		break;
		
		default: break;
	}

	if (smag == AliMagF::k2kG) {
		comment	 = comment.Append(" | L3 field 0.2 T");
	} else if (smag == AliMagF::k5kG) {
		comment = comment.Append(" | L3 field 0.5 T");
	}
	
	
	if (srad == kGluonRadiation){
		comment = comment.Append(" | Gluon Radiation On");
	
	} else {
		comment = comment.Append(" | Gluon Radiation Off");
	}

	printf("\n \n Comment: %s \n \n", comment.Data());
	
	
	// Field
	TGeoGlobalMagField::Instance()->SetField(new AliMagF("Maps","Maps", -1., -1., smag));

	rl->CdGAFile();
	//
	Int_t   iABSO   = 1;
	Int_t   iDIPO   = 1;
	Int_t   iFMD    = 1;
	Int_t   iFRAME  = 1;
	Int_t   iHALL   = 1;
	Int_t   iITS    = 1;
	Int_t   iMAG    = 1;
	Int_t   iMUON   = 1;
	Int_t   iPHOS   = 1;
	Int_t   iPIPE   = 1;
	Int_t   iPMD    = 1;
	Int_t   iHMPID  = 1;
	Int_t   iSHIL   = 1;
	Int_t   iT0     = 1;
	Int_t   iTOF    = 1;
	Int_t   iTPC    = 1;
	Int_t   iTRD    = 1;
	Int_t   iZDC    = 0; //1 // RS not needed in pp
	Int_t   iEMCAL  = 1;
	Int_t   iVZERO  = 1;
	Int_t   iACORDE = 1;

	//=================== Alice BODY parameters =============================
	AliBODY *BODY = new AliBODY("BODY", "Alice envelop");


	if (iMAG){
		//=================== MAG parameters ============================
		// --- Start with Magnet since detector layouts may be depending ---
		// --- on the selected Magnet dimensions ---
		AliMAG *MAG = new AliMAG("MAG", "Magnet");
	}


	if (iABSO){
		//=================== ABSO parameters ============================
		AliABSO *ABSO = new AliABSOv3("ABSO", "Muon Absorber");
	}

	if (iDIPO)	{
		//=================== DIPO parameters ============================
		AliDIPO *DIPO = new AliDIPOv3("DIPO", "Dipole version 3");
	}

	if (iHALL){
		//=================== HALL parameters ============================
		AliHALL *HALL = new AliHALLv3("HALL", "Alice Hall");
	}


	if (iFRAME){
		//=================== FRAME parameters ============================
		AliFRAMEv2 *FRAME = new AliFRAMEv2("FRAME", "Space Frame");
		FRAME->SetHoles(1);
	}

	if (iSHIL){
		//=================== SHIL parameters ============================
		AliSHIL *SHIL = new AliSHILv3("SHIL", "Shielding Version 3");
	}


	if (iPIPE)	{
		//=================== PIPE parameters ============================
		AliPIPE *PIPE = new AliPIPEv3("PIPE", "Beam Pipe");
	}

	if (iITS){
		//=================== ITS parameters ============================
		AliITS *ITS  = new AliITSv11("ITS","ITS v11");
	}

	if (iTPC){
	//============================ TPC parameters =====================
		AliTPC *TPC = new AliTPCv2("TPC", "Default");
	}


	if (iTOF) {
		//=================== TOF parameters ============================
		AliTOF *TOF = new AliTOFv6T0("TOF", "normal TOF");
	}


	if (iHMPID){
		//=================== HMPID parameters ===========================
		AliHMPID *HMPID = new AliHMPIDv3("HMPID", "normal HMPID");
	}


	if (iZDC)	{
		//=================== ZDC parameters ============================
		AliZDC *ZDC = new AliZDCv4("ZDC", "normal ZDC");
	}

	if (iTRD){
		//=================== TRD parameters ============================
		AliTRD *TRD = new AliTRDv1("TRD", "TRD slow simulator");
		AliTRDgeometry *geoTRD = TRD->GetGeometry();
		// Partial geometry: modules at 0,1,2,3,6,7,8,9,10,11,15,16,17
		// starting at 3h in positive direction
		geoTRD->SetSMstatus(4,0);
		geoTRD->SetSMstatus(5,0);
		geoTRD->SetSMstatus(12,0);
		geoTRD->SetSMstatus(13,0);
		geoTRD->SetSMstatus(14,0);
	}

	if (iFMD){
		//=================== FMD parameters ============================
		AliFMD *FMD = new AliFMDv1("FMD", "normal FMD");
	}

	if (iMUON){
		//=================== MUON parameters ===========================
		// New MUONv1 version (geometry defined via builders)
		AliMUON *MUON = new AliMUONv1("MUON", "default");
	}
	//=================== PHOS parameters ===========================

	if (iPHOS){
		AliPHOS *PHOS = new AliPHOSv1("PHOS", "noCPV_Modules123");
	}


	if (iPMD){
		//=================== PMD parameters ============================
		AliPMD *PMD = new AliPMDv1("PMD", "normal PMD");
	}

	if (iT0){
		//=================== T0 parameters ============================
		AliT0 *T0 = new AliT0v1("T0", "T0 Detector");
	}

	if (iEMCAL){
		//=================== EMCAL parameters ============================
		AliEMCAL *EMCAL = new AliEMCALv2("EMCAL", "EMCAL_COMPLETE12SMV1");
	}

	if (iACORDE){
		//=================== ACORDE parameters ============================
		AliACORDE *ACORDE = new AliACORDEv1("ACORDE", "normal ACORDE");
	}

	if (iVZERO){
		//=================== VZERO parameters ============================
		AliVZERO *VZERO = new AliVZEROv7("VZERO", "normal VZERO");
	}         
}

Float_t EtaToTheta(Float_t arg){
  return (180./TMath::Pi())*2.*atan(exp(-arg));
}



AliGenerator* GeneratorFactory(PprRun_t srun) {
	Int_t isw = 3;
	if (srad == kNoGluonRadiation) isw = 0;
	
	AliGenerator * gGener = 0x0;
	switch (srun) {
		case kPythia6:
			{
				comment = comment.Append(":Pythia6 perugia0 p-p");
				AliGenPythia *gener = new AliGenPythia(-1); 
				gener->SetMomentumRange(0,999999);
				gener->SetThetaRange(0., 180.);
				gener->SetYRange(-12,12);
				gener->SetPtRange(0,1000);
				gener->SetProcess(kPyMb);
				gener->SetEnergyCMS(energy);
				gener->SetProjectile("p", 1, 1) ; 
				gener->SetTarget("p", 1, 1) ; 
				gGener=gener;
			}
			break;
		case kPythia8:
			{
				comment = comment.Append(":Pythia8 p-p");
				AliGenPythiaPlus* gener = new AliGenPythiaPlus(AliPythia8::Instance());
				gener->SetProcess(kPyMbDefault);
				//   Centre of mass energy
				gener->SetEnergyCMS(energy);
				//   Initialize generator
				gener->SetEventListRange(-1, 2); 
				(AliPythia8::Instance())->ReadString("Random:setSeed = on");
				(AliPythia8::Instance())->ReadString(Form("Random:seed = %ld", seed%900000000)); 
				gGener=gener;
			}
			break;
		case kPythia8PlusSignals:
			{
				comment = comment.Append(":Pythia8 p-p + pi0 and eta signals");
				
				AliGenCocktail *cocktail = new AliGenCocktail();
				
				AliGenPythiaPlus* py8 = new AliGenPythiaPlus(AliPythia8::Instance());
				py8->SetProcess(kPyMbDefault);
				//   Centre of mass energy
				py8->SetEnergyCMS(energy);
				//   Initialize generator
				py8->SetEventListRange(-1, 2);
				(AliPythia8::Instance())->ReadString("Random:setSeed = on");
				(AliPythia8::Instance())->ReadString(Form("Random:seed = %ld", seed%900000000));
				
				cocktail->AddGenerator(py8,"pythia8",1);
				
				//
				// Pi0
				// Flat pt spectrum in range 0..45, 10 pi0
				// Set pseudorapidity range from -1.2 to 1.2
				//
				AliGenBox *pi0 = new AliGenBox(12);
				pi0->SetPart(111);
				pi0->SetPtRange(0.,45.);
				pi0->SetPhiRange(0,360);
				pi0->SetYRange(1.2,-1.2);
				cocktail->AddGenerator(pi0,"pi0", 1);
				
				//
				// Eta
				// Flat pt spectrum in range 0..45, 10 eta
				// Set pseudorapidity range from -1.2 to 1.2
				//
				AliGenBox *eta = new AliGenBox(12);
				eta->SetPart(221);
				eta->SetPtRange(0.,45.);
				eta->SetPhiRange(0,360);
				eta->SetYRange(1.2,-1.2);
				cocktail->AddGenerator(eta,"eta", 1);

				//
				// Omega
				// Flat pt spectrum in range 0..45, 10 omega
				// Set pseudorapidity range from -1.2 to 1.2
				//				
				AliGenBox *omega = new AliGenBox(12);
				omega->SetPart(223);
				omega->SetPtRange(0.,45.);
				omega->SetPhiRange(0,360);
				omega->SetYRange(1.2,-1.2);
				cocktail->AddGenerator(omega,"omega", 1);

				
				gGener=cocktail;
			}
			break;
		case kPhojet:
			{
				comment = comment.Append(":Phojet p-p, Diffractive Tune");
				//    DPMJET
				#if defined(__CINT__)
				gSystem->Load("libdpmjet");      // Parton density functions
				gSystem->Load("libTDPMjet");      // Parton density functions
				#endif
				AliGenDPMjet* gener = new AliGenDPMjet(-1);
				gener->SetMomentumRange(0, 999999.);
				gener->SetThetaRange(0., 180.);
				gener->SetYRange(-12.,12.);
				gener->SetTuneForDiff();
				gener->SetPtRange(0,1000.);
				gener->SetProcess(kDpmMb);
				gener->SetEnergyCMS(energy);
				//gener->SetCrossingAngle(0,0.000280);
				gGener=gener;
			}
			break;  
		default: break;
	}
	
	return gGener;
}


void ProcessEnvironmentVars()
{
    // Run type
    if (gSystem->Getenv("CONFIG_RUN_TYPE")) {
      for (Int_t iRun = 0; iRun < kRunMax; iRun++) {
			if (strcmp(gSystem->Getenv("CONFIG_RUN_TYPE"), pprRunName[iRun])==0) {
			srun = (PprRun_t)iRun;
			cout<<"Run type set to "<<pprRunName[iRun]<<endl;
			}
      }
    }

    // Random Number seed
    if (gSystem->Getenv("CONFIG_SEED")) {
      sseed = atoi(gSystem->Getenv("CONFIG_SEED"));
    }
    
    // Energy
    if (gSystem->Getenv("CONFIG_ENERGY")) {
      energy = atoi(gSystem->Getenv("CONFIG_ENERGY"));
      cout<<"Energy set to "<<energy<<" GeV"<<endl;
    }

    // Run number
    if (gSystem->Getenv("DC_RUN")) {
      runNumber = atoi(gSystem->Getenv("DC_RUN"));
    }

}

