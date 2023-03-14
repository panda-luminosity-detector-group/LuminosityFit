// Panda FullSim macro

#ifndef __CLING__

#include <PndCave.h>
#include <PndDpmGenerator.h>
#include <PndLmdDetector.h>
#include <PndLmdHybridHitProducer.h>
#include <PndMagnet.h>
#include <PndMultiField.h>
#include <PndMultiFieldPar.h>
#include <PndPipe.h>
#include <PndSdsGeoPar.h>
#include <PndBoxGenerator.h>
#include "FairFilteredPrimaryGenerator.h"

#include <FairBoxGenerator.h>
#include <FairFileSource.h>
#include <FairParAsciiFileIo.h>
#include <FairParRootFileIo.h>
#include <FairPrimaryGenerator.h>
#include <FairRunAna.h>
#include <FairRunSim.h>

#include <TGeant4.h>
#include <TStopwatch.h>

#include "TROOT.h"
#include "TFile.h"

#include <iostream>
#include <string>

#endif

int runLumiPixel0SimBoxRestGas(const int nEvents = 100, const int startEv = 0, TString storePath = "tmpOutput", const int verboseLevel = 0, const int particle = -2212, double mom = 15,
                        const double beam_X0 = 0.0, const double beam_Y0 = 0.0, const double target_Z0 = 0.0, const double beam_width_sigma_X = 0.0,
                        const double beam_width_sigma_Y = 0.0,
                        const double target_width_Z = 0.0, // beam offset and smearing parameters
                        const double beam_grad_X = 0.0, const double beam_grad_Y = 0.0, const double beam_grad_sigma_X = 0.0,
                        const double beam_grad_sigma_Y = 0.0, // beam gradient parameters
                        const int trkNum = 1, const int seed = 0, const double dP = 0, const TString lmd_geometry_filename = "Luminosity-Detector.root",
                        std::string misalignment_matrices_path = "", bool use_point_transform_misalignment = false)
{

  gSystem->Load("LmdSensorAlignerTools");

  gRandom->SetSeed(seed);
  TStopwatch timer;
  timer.Start();
  gDebug = 0;
  mom += dP;
  cout << "We start run for beam Mom = " << mom << endl;
  // output1
  TString simOutput = storePath + "/Lumi_MC_";
  simOutput += startEv;
  simOutput += ".root";
  TString parOutput = storePath + "/Lumi_Params_";
  parOutput += startEv;
  parOutput += ".root";

  FairRunSim *fRun = new FairRunSim();
  cout << "All libraries succsesfully loaded!" << endl;

  // set the MC version used
  fRun->SetName("TGeant4");
  // fRun->SetName("TGeant3");  //GEANE uses GEANT3!

  fRun->SetOutputFile(simOutput);

  // set material
  fRun->SetMaterials("media_pnd.geo");

  // create and add detectors
  FairModule *Cave = new PndCave("CAVE");
  Cave->SetGeometryFileName("pndcave.geo");
  fRun->AddModule(Cave);

  FairModule *Pipe = new PndPipe("PIPE");
  Pipe->SetGeometryFileName("beampipe_201309.root");
  fRun->AddModule(Pipe);

  FairModule *Magnet = new PndMagnet("MAGNET");
  Magnet->SetGeometryFileName("FullSolenoid_V842.root");
  fRun->AddModule(Magnet);

  FairModule *Dipole = new PndMagnet("MAGNET");
  Dipole->SetGeometryFileName("dipole.geo");
  fRun->AddModule(Dipole);

  PndLmdDetector *Lum = new PndLmdDetector("LUM", kTRUE);
  Lum->SetExclusiveSensorType("LumActive"); // ignore MVD
  Lum->SetGeometryFileName(lmd_geometry_filename);
  Lum->SetVerboseLevel(verboseLevel);

  fRun->AddModule(Lum);

  // particle generator
  TString tgtfile = gSystem->Getenv("VMCWORKDIR");
  FairFilteredPrimaryGenerator *primGen = new FairFilteredPrimaryGenerator();
  primGen->SetBeam(beam_X0, beam_Y0, 0.0, 0.0); 
  primGen->SetBeamAngle(beam_grad_X, beam_grad_Y, beam_grad_sigma_X, beam_grad_sigma_Y);
  //fRun->SetGenerator(primGen);

  // Box Generator
  //FairBoxGenerator *fBox = new FairBoxGenerator(particle, trkNum);
  PndBoxGenerator *fBox = new PndBoxGenerator(particle, trkNum);
  fBox->SetPRange(mom, mom);
  //  fBox->SetThetaRange(0.52,0.63); // 9 ... 11 mrad
  //  fBox->SetThetaRange(0.12,0.7); // 2... 12 mrad
    fBox->SetThetaRange(0.12,0.8); // 2... 14 mrad
  //  fBox->SetThetaRange(1.3,1.4); // mrad outside of the detector geometry by purpose
  //fBox->SetThetaRange(0.1547, 180.); // 2.7 mrad... pi rad
  // fBox->SetThetaRange(0.12,0.65); // 2... 11 mrad
  // fBox->SetThetaRange(0.229183, 0.458366); //4 ... 8 mrad
  // fBox->SetThetaRange(0.229183,0.31512);//4..5.5 mrad
  // fBox->SetThetaRange(0.229,0.229);//4..mrad
  // fBox->SetPhiRange(90,90.);
  fBox->SetPhiRange(0, 360.);
  // fBox->SetPhiRange(0.5,359.5); //FOR missed track check
  primGen->AddGenerator(fBox);

  cout << "Begin using Rest Gas Target!" <<endl;
  tgtfile += "/input/restgasthickness_210421_Normal_IP_with_Cryopump.txt";
  cout << tgtfile <<endl;
  LOG(info) << "Using distributed Beam-Target profile " << tgtfile.Data();
  TObjArray *genList = primGen->GetListOfGenerators();
  cout << genList->GetEntriesFast() <<endl;
  for (int i = 0; i < genList->GetEntriesFast(); i++) {
    TObject *obj = genList->At(i);
    if (!obj->InheritsFrom("PndTargetGenerator"))
      continue;
    PndTargetGenerator *aGen = (PndTargetGenerator *)genList->At(i);
    //aGen->SetBeam(beam_X0, beam_Y0, beam_width_sigma_X, beam_width_sigma_Y);
    aGen->SetDensityProfile(tgtfile);
    aGen->SetBeamRadius(0.1);                 // default beam spot sigma 1mm^2 by "Fair Operation Modes" document v.6 (2020)
    aGen->SetBeamPipeRadius(2.0);             // Smallest radius around interaction region
    aGen->SetConstantBeamRegion(-140., 223.); // region where the beam with stays at its minimum due to the solenoid field, edges at half Bz_max
    aGen->SetBeamDrDz(0.1);                   // default maximum beam divergence from 4-sigma emittance of 1-2 mm mrad by "Fair Operation Modes" document v.6 (2020)
    aGen->ReadDensityFile();
//    fGen->AddGenerator(aGen);
    cout << "Rest Gas Target is used !" <<endl;
  }

  // reading the new field map in the old format
  fRun->SetGenerator(primGen);
  fRun->SetBeamMom(mom);

  PndMultiField *fField = new PndMultiField("AUTO");

  fRun->SetField(fField);

  if (nEvents < 101)
    fRun->SetStoreTraj(kTRUE); // toggle this for use with EVE
  else
    fRun->SetStoreTraj(kFALSE);

  // set misalignement matricies
  std::map<std::string, TGeoHMatrix> *matrices;
  if (misalignment_matrices_path != "" && !use_point_transform_misalignment) {
    cout << "INFO: using misalignment matrices " << misalignment_matrices_path << "\n";
    std::map<std::string, TGeoHMatrix> *matrices = PndMatrixUtil::readMatrices(misalignment_matrices_path);
    cout << matrices->size() << " matrices successfully read from file.";

    // this call has to be made before fRun->Init();
    fRun->AddAlignmentMatrices(*matrices);
  }

  fRun->Init();

//  ((TGeant4 *)gMC)->ProcessGeantCommand("/mcVerbose/eventAction 0");
//  ((TGeant4 *)gMC)->ProcessGeantCommand("/mcTracking/loopVerbose 0");

  // Fill the Parameter containers for this run
  //-------------------------------------------
  FairRuntimeDb *rtdb = fRun->GetRuntimeDb();
  Bool_t kParameterMerged = kTRUE;
  FairParRootFileIo *output = new FairParRootFileIo(kParameterMerged);
  output->open(parOutput.Data(), "RECREATE");
  rtdb->setOutput(output);
  PndMultiFieldPar *Par = (PndMultiFieldPar *)rtdb->getContainer("PndMultiFieldPar");
  if (fField) {
    Par->SetParameters(fField);
  }
  Par->setInputVersion(fRun->GetRunId(), 1);
  Par->setChanged();

  // Transport nEvents
  fRun->Run(nEvents);
  rtdb->saveOutput();
  // rtdb->print();

  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  printf("RealTime=%f seconds, CpuTime=%f seconds\n", rtime, ctime);

  // temporary fix to avoid double frees at the destruction of te program for pandaroot/fairroot with root6
  gGeoManager->GetListOfVolumes()->Delete();
  gGeoManager->GetListOfShapes()->Delete();
  delete gGeoManager;

  return 0;
}
