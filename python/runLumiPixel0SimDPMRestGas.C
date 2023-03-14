#ifndef __CLING__

#include <PndCave.h>
#include <PndDpmGenerator.h>
#include <PndLmdDetector.h>
#include <PndMagnet.h>
#include <PndMultiField.h>
#include <PndPipe.h>

#include <FairParRootFileIo.h>
#include <FairPrimaryGenerator.h>
#include <FairRunSim.h>
#include "FairFilteredPrimaryGenerator.h"
#include "PndFilteredPrimaryGenerator.h"

#include <TGeant4.h>
#include <TStopwatch.h>

#include "TROOT.h"
#include "TFile.h"


#include <iostream>
#include <string>

#endif

using std::cout;
using std::string;

// Lmd DPM Sim macro
//int runLumiPixel0SimDPM(const int nEvents = 10, const int startEvent = 0, const double mom = 15, TString input = "input.root", TString storePath = "tmpOutputDPM",
//                        const double beam_X0 = 0.0, const double beam_Y0 = 0.0, const double target_Z0 = 0.0, const double beam_width_sigma_X = 0.0,
//                        const double beam_width_sigma_Y = 0.0,
//                        const double target_width_Z = 0.0, // beam offset and smearing parameters
//                        const double beam_grad_X = 0.0, const double beam_grad_Y = 0.0, const double beam_grad_sigma_X = 0.0,
//                        const double beam_grad_sigma_Y = 0.0, // beam gradient parameters
//                        const TString lmd_geometry_filename = "Luminosity-Detector.root", std::string misalignment_matrices_path = "",
//                        bool use_point_transform_misalignment = false, const int verboseLevel = 3)
int runLumiPixel0SimDPMRestGas(const int nEvents = 10, const int startEvent = 0, const double mom = 15, TString input = "input.root", TString storePath = "tmpOutputDPM",
                        const double beam_X0 = 0.0, const double beam_Y0 = 0.0, const double target_Z0 = 0.0, const double beam_width_sigma_X = 0.0,
                        const double beam_width_sigma_Y = 0.0,
                        const double target_width_Z = 0.0, // beam offset and smearing parameters
                        const double beam_grad_X = 0.0, const double beam_grad_Y = 0.0, const double beam_grad_sigma_X = 0.0,
                        const double beam_grad_sigma_Y = 0.0, // beam gradient parameters
                        const TString lmd_geometry_filename = "Luminosity-Detector.root", std::string misalignment_matrices_path = "",
                        bool use_point_transform_misalignment = false, const int verboseLevel = 3)
{
  // gRandom->SetSeed(seed);
  Int_t mode = 2;
  TStopwatch timer;
  timer.Start();
  gDebug = 0;

  // output1
  TString simOutput = storePath + "/Lumi_MC_";
  simOutput += startEvent;
  simOutput += ".root";
  TString parOutput = storePath + "/Lumi_Params_";
  parOutput += startEvent;
  parOutput += ".root";

  FairRunSim *fRun = new FairRunSim();

  // set the MC version used
  fRun->SetName("TGeant4");

  fRun->SetOutputFile(simOutput);

  // set material
  fRun->SetMaterials("media_pnd.geo");

  fRun->SetGenerateRunInfo(false);
  fRun->SetUseFairLinks(true);
  //  //create and add detectors
  // //-------------------------  CAVE      -----------------

  FairModule *Cave = new PndCave("CAVE");
  Cave->SetGeometryFileName("pndcave.geo");
  fRun->AddModule(Cave);
  //-------------------------  Magnet   -----------------
  // this part is commented because the solenoid magnet is contained in MDT geo
  FairModule *Magnet = new PndMagnet("MAGNET");
  Magnet->SetGeometryFileName("FullSuperconductingSolenoid_v831.root");
  fRun->AddModule(Magnet);

  FairModule *Dipole = new PndMagnet("MAGNET");
  Dipole->SetGeometryFileName("dipole.geo");
  fRun->AddModule(Dipole);
  //-------------------------  Pipe     -----------------
  FairModule *Pipe = new PndPipe("PIPE");
  Pipe->SetGeometryFileName("beampipe_201309.root");
  fRun->AddModule(Pipe);

  PndLmdDetector *Lum = new PndLmdDetector("LUM", kTRUE);
  //	Lum->SetExclusiveSensorType("LumActive"); //ignore MVD
  Lum->SetGeometryFileName(lmd_geometry_filename);
  Lum->SetVerboseLevel(verboseLevel);
  fRun->AddModule(Lum);

  // particle generator
  TString tgtfile = gSystem->Getenv("VMCWORKDIR");
  FairFilteredPrimaryGenerator *fGen = new FairFilteredPrimaryGenerator();
  fGen->SetBeam(beam_X0, beam_Y0, 0.0, 0.0);
  fGen->SetBeamAngle(beam_grad_X, beam_grad_Y, beam_grad_sigma_X, beam_grad_sigma_Y);
  
  // DPM Generator
  // PndDpmGenerator *dpmGen = new PndDpmGenerator(input);
  // fGen->AddGenerator(dpmGen);
   PndDpmDirect *dpmGen = new PndDpmDirect(mom, mode, gRandom->GetSeed(), 0.1547);
   fGen->AddGenerator(dpmGen);


//  if(UseRestGas)
//  {
    cout << "Begin using Rest Gas Target!" <<endl;
    tgtfile += "/input/restgasthickness_210421_Normal_IP_with_Cryopump.txt";
    cout << tgtfile <<endl;
    LOG(info) << "Using distributed Beam-Target profile " << tgtfile.Data();
    TObjArray *genList = fGen->GetListOfGenerators();
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
//      fGen->AddGenerator(aGen);
      cout << "Rest Gas Target is used !" <<endl;
    }
//  }

/*  else{
    FairPrimaryGenerator *primGen = new FairPrimaryGenerator();
    if (beam_X0 != 0.0 || beam_Y0 != 0.0 || beam_width_sigma_X > 0.0 || beam_width_sigma_Y > 0.0) {
      primGen->SmearGausVertexXY(true);
      primGen->SetBeam(beam_X0, beam_Y0, beam_width_sigma_X, beam_width_sigma_Y);
    }
    if (target_Z0 != 0.0 || target_width_Z > 0.0) {
      primGen->SmearGausVertexZ(true);
      primGen->SetTarget(target_Z0, target_width_Z);
    }
    if (beam_grad_X != 0.0 || beam_grad_Y != 0.0 || beam_grad_sigma_X > 0.0 || beam_grad_sigma_Y > 0.0) {
      primGen->SetBeamAngle(beam_grad_X, beam_grad_Y, beam_grad_sigma_X, beam_grad_sigma_Y);
    }
  }
*/

  fRun->SetGenerator(fGen);

  // reading the new field map in the old format
  fRun->SetBeamMom(mom);
  PndMultiField *fField = new PndMultiField("AUTO");
  fRun->SetField(fField);

  fRun->SetStoreTraj(false); // toggle this for use with EVE

  // set misalignement matricies
  std::map<std::string, TGeoHMatrix> *matrices;
  // TODO: replace with generalize call from either martix handler or AlignmentHandler
  if (misalignment_matrices_path != "" && !use_point_transform_misalignment) {
    cout << "INFO: using misalignment matrices " << misalignment_matrices_path << "\n";
    std::map<std::string, TGeoHMatrix> *matrices =PndMatrixUtil::readMatrices(misalignment_matrices_path);
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

  // Transport nEvents
  // -----------------
  fRun->Run(nEvents);
  rtdb->saveOutput();
  // rtdb->print();

  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  printf("RealTime=%f seconds, CpuTime=%f seconds\n", rtime, ctime);

  return 0;
}
