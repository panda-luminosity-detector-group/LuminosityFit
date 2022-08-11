#include "KoaCombinedDataReader.h"
#include "KoaMCTrack.h"
#include "PndSdsMCPoint.h"
#include "PndSdsHit.h"
#include "PndTrack.h"
#include "FairTrackParH.h"

KoaCombinedDataReader::KoaCombinedDataReader()
    : rel_momentum_deviation_threshold(0.0003), data_tree("koalasim"),
      filtered_track_array("Koa_IP", 100) {
  comp_array = 0;
  koa_array = 0;
  MCtrack_array = 0;
  MCpoint_array = 0;
}

KoaCombinedDataReader::~KoaCombinedDataReader() {}

unsigned int KoaCombinedDataReader::getEntries() const {
  return data_tree.GetEntries();
}

void KoaCombinedDataReader::initDataStream() {
  for (unsigned int i = 0; i < file_paths.size(); i++)
    data_tree.Add(file_paths[i]);

  comp_array = new TClonesArray("PndTrack");
  data_tree.SetBranchAddress("LMDComp", &comp_array);
  data_tree.SetBranchStatus("LMDComp*", 1);

  koa_array = new TClonesArray("PndSdsHit");
  data_tree.SetBranchAddress("KOALAComp", &koa_array);
  data_tree.SetBranchStatus("KOALAComp*", 1);

  MCtrack_array = new TClonesArray("KoaMCTrack");
  data_tree.SetBranchAddress("MCTrack", &MCtrack_array);
  data_tree.SetBranchStatus("MCTrack*", 1);

  MCpoint_array = new TClonesArray("PndSdsMCPoint");
  data_tree.SetBranchAddress("LMDMCPoint", &MCpoint_array);
  data_tree.SetBranchStatus("LMDMCPoint*", 1);
}

void KoaCombinedDataReader::clearDataStream() {}

std::vector<Lmd::Data::TrackPairInfo>
KoaCombinedDataReader::getEntry(unsigned int i) {
  data_tree.GetEntry(i);
  filtered_track_array.Clear();
  unsigned int good_track_counter = 0;

  std::vector<Lmd::Data::TrackPairInfo> track_pairs;

  // loop over track array
  for (Int_t iN = 0; iN < comp_array->GetEntries(); iN++) {
    PndTrack* trcnd = (PndTrack *)comp_array->At(iN);
    FairTrackParP numPts = trcnd->GetParamFirst();
    TVector3 temp_vec;

    Lmd::Data::TrackPairInfo track_info;

    //this is for Lumi
    double HitX = numPts.GetX();
    double HitY = numPts.GetY();
    double HitZ = numPts.GetZ();
    double LmdPX = numPts.GetPx();
    double LmdPZ = numPts.GetPz();
    double LmdPY = numPts.GetPy();

    double lx = HitX + LmdPX;    //gives second point of the track in x
    double lz = HitZ + LmdPZ;
    TVector3 v1(LmdPX,LmdPY,LmdPZ);
    double lmdthet = v1.Theta();
    double lmdphi = v1.Phi();

    //this is for KOALA

    for (unsigned int tracki = 0; tracki < koa_array->GetEntries(); tracki++) {

	    PndSdsHit* Lkocomp = (PndSdsHit*)koa_array->At(tracki);
 	    double suX = Lkocomp->GetX();
	    double suZ = Lkocomp->GetZ();
	    double koathet = 1.559 - lmdthet; //angle between fowards and backwards particle is a little bit smaller than ninety°
	    double kPX = sin(koathet);
	    double kPZ = cos(koathet);

	    double kx = suX + kPX;    //gives second point on the track in X
	    double kz = suZ + kPZ;


	    double IPp1 = HitZ - lz;      //This are all terms for the calculation of a cut between two lines in two dimensions
	    double IPp2 = (kx * suZ) - ( suX * kz);
	    double IPp3 = suZ - kz;
	    double IPp4 = (lx * HitZ) - ( HitX * lz);
	    double IPp5 = kz - suZ;
	    double IPp6 = lx - HitX;
	    double IPp7 = lz - HitZ;
	    double IPp8 = kx - suX;

            double IPzz = (IPp1 * IPp2) - (IPp3 * IPp4);
	    double IPzn = ( IPp5 * IPp6) - (IPp7 - IPp8);
            double IPz = IPzz / IPzn;

	    //gives Time it took for the particle to reach the sensor
	    double dist = (HitZ - IPz)/LmdPZ;

	    //gives x,y and z
	    double pocaX = HitX - dist * LmdPX;
	    double pocaY = HitY - dist * LmdPY;
	    double pocaZ = HitZ - dist * LmdPZ;


    
      track_info.IsReconstructedAtLmd = true;
      track_info.IsReconstructedAtIP = true;
      track_info.IsSecondary = false;

    if (track_info.IsReconstructedAtIP) {
       track_info.RecoIP.Position = {pocaX, pocaY, pocaZ};
       track_info.RecoIP.Momentum = {LmdPX, LmdPY, LmdPZ};
    }

    if (track_info.IsReconstructedAtLmd) {
      track_info.RecoLMD.Position = {HitX, HitY,HitZ};
      track_info.RecoLMD.Momentum = {LmdPX, LmdPY, LmdPZ};
    }

    for (unsigned int iMC = 0; iMC < MCtrack_array->GetEntries(); iMC++){
	    KoaMCTrack* MCPos = (KoaMCTrack*)MCtrack_array->At(iMC);

	    double pZ = MCPos->GetPz();
	    double sX = MCPos->GetStartX();
	    double sY = MCPos->GetStartY();
	    double sZ = MCPos->GetStartZ();

	    if(pZ>0.5){
		    
		    track_info.MCIP.Position = {sX, sY, sZ};
	    }
    }

     for (unsigned int iMCP = 0; iMCP < MCpoint_array->GetEntries(); iMCP++){
	     PndSdsMCPoint* MCPoi = (PndSdsMCPoint*)MCpoint_array->At(iMCP);

	     double LmdpX = MCPoi->GetPx();
	     double LmdpY = MCPoi->GetPy();
	     double LmdpZ = MCPoi->GetPz();
	     double LmdX = MCPoi->GetX();
	     double LmdY = MCPoi->GetY();
	     double LmdZ = MCPoi->GetZ();

	     if(LmdZ<530){


	
		     track_info.MCIP.Momentum = {LmdpX, LmdpY, LmdpZ};

		     
		     track_info.MCLMD.Position = {LmdX, LmdY, LmdZ};
		     track_info.MCLMD.Momentum = {LmdpX, LmdpY, LmdpZ};
	     }
     }
         
    }

    track_pairs.push_back(track_info);
  }

  return track_pairs;
}
