void sim(Int_t nev=3) {
  AliSysInfo::SetVerbose(kTRUE);
  AliSimulation simulator;

  simulator.SetMakeDigits("ITS TPC TRD TOF PHOS HMPID EMCAL MUON FMD PMD T0 VZERO");
  simulator.SetMakeSDigits("TRD TOF PHOS HMPID EMCAL MUON ZDC PMD T0 VZERO FMD");
  simulator.SetMakeDigitsFromHits("ITS TPC");
  
  //
  simulator.SetRunQA(":");
  //
  // RAW OCDB
  simulator.SetDefaultStorage("alien://Folder=/alice/data/2012/OCDB");

  // Specific storages = 23

  //
  // ITS  (1 Total)
  //     Alignment from Ideal OCDB

  simulator.SetSpecificStorage("ITS/Align/Data",  "alien://Folder=/alice/simulation/2008/v4-15-Release/Ideal");

  //
  // MUON (1 object)

  simulator.SetSpecificStorage("MUON/Align/Data","alien://folder=/alice/simulation/2008/v4-15-Release/Ideal");

  //
  // TPC (6 total)

  simulator.SetSpecificStorage("TPC/Calib/TimeGain",       "alien://Folder=/alice/simulation/2008/v4-15-Release/Ideal/");
  simulator.SetSpecificStorage("TPC/Calib/ClusterParam",   "alien://Folder=/alice/simulation/2008/v4-15-Release/Ideal/");
  simulator.SetSpecificStorage("TPC/Calib/AltroConfig",    "alien://Folder=/alice/simulation/2008/v4-15-Release/Ideal/");
  simulator.SetSpecificStorage("TPC/Calib/Correction",     "alien://Folder=/alice/simulation/2008/v4-15-Release/Ideal/");
  simulator.SetSpecificStorage("TPC/Align/Data",           "alien://Folder=/alice/simulation/2008/v4-15-Release/Ideal/");
  simulator.SetSpecificStorage("TPC/Calib/TimeDrift",      "alien://Folder=/alice/simulation/2008/v4-15-Release/Ideal/");
  simulator.SetSpecificStorage("TPC/Calib/Goofie",         "alien://Folder=/alice/simulation/2008/v4-15-Release/Ideal/");
  simulator.SetSpecificStorage("TPC/Calib/HighVoltage",    "alien://Folder=/alice/simulation/2008/v4-15-Release/Ideal/");
  simulator.SetSpecificStorage("TPC/Calib/RecoParam",      "alien://Folder=/alice/simulation/2008/v4-15-Release/Residual/"); 
  //

  // Vertex and magfield

  simulator.UseVertexFromCDB();
  simulator.UseMagFieldFromGRP();

  // PHOS simulation settings
  AliPHOSSimParam *simParam = AliPHOSSimParam::GetInstance();
  simParam->SetCellNonLineairyA(0.001);
  simParam->SetCellNonLineairyB(0.2);
  simParam->SetCellNonLineairyC(1.02);

  //
  // The rest
  //
  printf("Before simulator.Run(nev);\n");
  simulator.Run(nev);
  printf("After simulator.Run(nev);\n");
}
 
