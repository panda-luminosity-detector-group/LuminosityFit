//  edition 2.03.2012  A.Galoyan
//  main.cc,v 1.18 2004/03/04 15:09:11 ritman Exp $
//
// C++ program to call DPM event generator for PBAP P interactions 
// Original author:   A.Galoyan        
// change to TParticle output format J.Ritman May 2003
/* ------------------------------------------------------- */

#include <iostream>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include "TROOT.h"
#include "TFile.h"
#include "TClonesArray.h"
#include "TTree.h"
#include "TStopwatch.h"
#include "TParticle.h"

using namespace std;

extern "C" struct {
  long int n, k[2000];  
  double p[5000];   	
} lujets_;

// n   - number of produced particles, 
// k[] - Pythia particle identifiers
// p[] - kinematical characteristics of particles

extern "C" int init1_(double* Plab, double* seed, double* Elastic, double* tetmin); // to install DPM generator
extern "C" int dpm_gen_(double* Generator, double* seed); //to generate events

int convertStringToUInt(char *s, unsigned int &i) {
  char* p = s;
  errno = 0;
  i = strtoul(s, &p, 10);
  if (errno != 0) {
    std::cerr<<"conversion failed (EINVAL, ERANGE)\n";
    return 1;
  }
  if (s == p) {
    std::cerr<<"conversion failed (no characters consumed)\n";
    return 1;
  }
  if (*p != 0) {
    std::cerr<<"conversion failed (trailing data)\n";
    return 1;
  }
  return 0;
}

int convertStringToDouble(char *s, double &d) {
  char* p = s;
  errno = 0;
  d = strtod(s, &p);
  if (errno != 0) {
    std::cerr<<"conversion failed (EINVAL, ERANGE)\n";
    return 1;
  }
  if (s == p) {
    std::cerr<<"conversion failed (no characters consumed)\n";
    return 1;
  }
  if (*p != 0) {
    std::cerr<<"conversion failed (trailing data)\n";
    return 1;
  }
  return 0;
}

int main(int argc, char* argv[])
{
  double Plab, tetmin;          // Plab - PBAP momentum in Lab.Sys. 
  double seed;
  unsigned int ntot, Ieven, npart, i, Elastic;	
  double Px[1000],Py[1000],Pz[1000],E[1000],Pm[1000],Wh[1000];
  int Id[1000];
  //	Elastic=0.;	 // No elastic scattering, only inelastic
  Elastic=1;       // Elastic and inelastic interactions
  //	Elastic=2.;	 // Only elastic scattering, no inelastic one
  char rootfile_name[200] = "Background-micro.root";

  // ok first of all parse arguments
  int seed_flag =0, mom_flag = 0, elastic_flag = 0, thetamin_flag = 0, num_flag = 0, file_flag = 0; 
  int c;
  char *seed_value = NULL, *mom_value = NULL, *elastic_value = NULL, *thetamin_value = NULL, *num_value = NULL, *file_value = NULL;

  while ((c = getopt (argc, argv, "hs:m:e:t:n:f:")) != -1) {
    switch (c)
    {
      case 'h':
	// display help and exit
	std::cout<<"Following options are available: \n";
	std::cout<<"-h flag: display this help "<<std::endl;
	std::cout<<"-s flag: random seed number "<<std::endl;
	std::cout<<"-m flag: beam momentum "<<std::endl;     
	std::cout<<"-e flag: reaction type (0 only inelastic; 1 elastic & inelastic; 2 only elastic) "<<std::endl;
	std::cout<<"-t flag: mininmal theta value "<<std::endl;
	std::cout<<"-n flag: number of events to simulate "<<std::endl;
	std::cout<<"-f flag: output root file name "<<std::endl;
	return 0;
      case 's':
        seed_flag = 1;
        seed_value = optarg;
        break;
      case 'm':
        mom_flag = 1;
        mom_value = optarg;
        break;
      case 'e':
        elastic_flag = 1;
        elastic_value = optarg;
        break;
      case 't':
        thetamin_flag = 1;
        thetamin_value = optarg;
        break;
      case 'n':
        num_flag = 1;
        num_value = optarg;
        break;
      case 'f':
        file_flag = 1;
        file_value = optarg;
        break;
      case '?':
        if (optopt == 's' || optopt == 'm' || optopt == 'e' || optopt == 't' || optopt == 'n' || optopt == 'f')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
              "Unknown option character `\\x%x'.\n",
              optopt);
        return 1;
      default:
        abort ();
    }
  }

  // ok now convert strings to floats and ints and set the correct values

  if(seed_flag) {
    if(convertStringToDouble(seed_value, seed))
      seed_flag = 0;
  }
  if(mom_flag) {
    if(convertStringToDouble(mom_value, Plab))
      mom_flag = 0;
  }
  if(elastic_flag) {
    if(convertStringToUInt(elastic_value, Elastic))
      elastic_flag = 0;
  }
  if(thetamin_flag) {
    if(convertStringToDouble(thetamin_value, tetmin))
      thetamin_flag = 0;
  }
  if(num_flag) {
    if(convertStringToUInt(num_value, ntot))
      num_flag = 0;
  }
  if(file_flag)
    sprintf(rootfile_name, "%s", file_value);

  //   Root initialization 
  //TROOT root("DPMGenerator","DPM background generator");
  TFile f1(rootfile_name,"RECREATE","ROOT_Tree"); 

  double Generator=0.;
  Double_t weight = 1.0;
  Int_t activeCnt=0;
  TTree* fTree = new TTree("data","DPM Background");
  TClonesArray* fEvt;

  fEvt=new TClonesArray("TParticle",100);
  fTree->Branch("Npart",&activeCnt,"Npart/I");
  fTree->Branch("Weigth",&weight,"Weight/D");
  fTree->Branch("Seed",&seed,"Weight/D");
  fTree->Branch("Particles",&fEvt, 32000,99);

  if(!seed_flag) {
    std::cout<<" Give as seed a large float number (eg. 123456.): ";
    std::cin>>seed;
  }
  if (!seed){  // if the seed is 0 then take the time
    Long_t Time = time(NULL);
    int a = Time/100000;
    seed = Time - a*100000 + a/100000.;
  }

  if(!mom_flag) {
    std::cout << " Enter  P_lab(GeV/c), ";   
    std::cin >> Plab ;    
  }

  if(!elastic_flag) {
    std::cout << " Enter  Elastic : 0., 1. or 2. " << "\n" << 
      "0. - No elastic scattering, only inelastic"<< "\n" <<
      "1. - Elastic and inelastic interactions" << "\n" <<     
      "2. - Only elastic scattering, no inelastic one"<< "\n";   
    std::cin >> Elastic ;
  }

  if((Elastic==1) || (Elastic==2)) {  
    if(!thetamin_flag) {
      std::cout << " Teta_min (degree) ";   
      std::cin >> tetmin; 
    }
  }
  else  {tetmin=0;}

  double Plabf, Elasticf, tetminf;
  Plabf = (double)Plab;
  Elasticf = (double)Elastic;
  tetminf = (double)tetmin;

  if(!num_flag) {
    std::cout << " Enter  N_Events ";
    std::cin >> ntot;
  }

  std::cout<<"Using following options: \n";
  std::cout<<"-s flag: random seed number: "<<seed<<std::endl;
  std::cout<<"-m flag: beam momentum: "<<Plabf<<"GeV/c"<<std::endl;     
  std::cout<<"-e flag: reaction type (0 only inelastic; 1 elastic & inelastic; 2 only elastic): "<<Elasticf<<std::endl;
  std::cout<<"-t flag: mininmal theta value: "<<tetminf<<" degrees"<<std::endl;
  std::cout<<"-n flag: number of events to simulate: "<<ntot<<std::endl;
  std::cout<<"-f flag: output root file name: "<<rootfile_name<<std::endl;

  std::cout<<"now we initialize the dpm generator...\n";
  init1_(&Plab,&seed,&Elasticf, &tetmin);  // installation of the DPM generator  
  
  TStopwatch timer;                        // time loop
  timer.Start();

  TLorentzVector Mom; 
  TLorentzVector V(0,0,0,0);

  // Simulation of events
  for (Ieven = 1; Ieven <= ntot; ++Ieven) {
    if( (Ieven%100) == 0 ) 
      std::cout << "Event number = " << Ieven << std::endl; 

    dpm_gen_(&Generator, &seed);
    fEvt->Clear();
    Int_t cnt = 0;

    // Loop over all produced particles 
    npart = lujets_.n;

    for (i= 0; i< npart; ++i) {        // update TClonesArrays

       /*    std::cout << i  <<"  " << lujets_.k[i+1000] << "   " 
      	  << lujets_.p[i] << "  " << lujets_.p[i+1000]  <<"   "
      	  << lujets_.p[i+2000] << "  " << lujets_.p[i+3000] << "   "
      	  << lujets_.p[i+4000]  << " \n" ;*/

      // i - order number of particle
      // lujets_.k[i+1000] - identifier of i-th particle
      // lujets_.p[i-1]        - Px (GeV/c) of i-th particle
      // lujets_.p[i+1000] - Py (GeV/c) of i-th particle
      // lujets_.p[i+2000] - Pz (GeV/c) of i-th particle
      // lujets_.p[i+3000] - Energy (GeV) of i-th particle
      // lujets_.p[i+4000] - Mass   (GeV) of i-th particle

      Id[i]=lujets_.k[i+1000];
      Px[i]=lujets_.p[i];
      Py[i]=lujets_.p[i+1000];
      Pz[i]=lujets_.p[i+2000];
      Pm[i]=lujets_.p[i+4000];
      E[i]=lujets_.p[i+3000];
      Wh[i]=1.0;

      Mom.SetPxPyPzE(Px[i],Py[i],Pz[i],E[i]);
      TParticle  fparticle(Id[i],1,0,0,0,0,Mom,V);
      new((*fEvt)[cnt++]) TParticle(fparticle);
    }
    activeCnt = cnt;

    fTree->Fill();                     //!
  } 

  timer.Stop(); 

  std::cout << " ----- Realtime: "<<timer.RealTime()<<"sec"<<std::endl;
  std::cout << " ----- Cputime:  "<<timer.CpuTime()<<"sec"<<std::endl;
  std::cout << " ----- Time/Event:"<<timer.CpuTime()/ntot<<"sec"<<std::endl;
  std::cout << " ----- Speed:    "<<ntot/timer.CpuTime()<<"Hz"<<std::endl;
  std::cout << std::endl;

  //-----------------------------------------------------------------

  f1.Write();    
  // f1.Close();
  // delete fTree;
}
