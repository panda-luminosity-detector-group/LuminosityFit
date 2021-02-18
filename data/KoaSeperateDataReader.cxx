#include "KoaSeperateDataReader.h"

#include "KoaMCTrack.h"
#include "PndSdsMCPoint.h"
#include "PndTrack.h"
#include "FairTrackParH.h"

KoaSeperateDataReader::KoaSeperateDataReader() :
		MC_tree("koalasim"), track_tree("koalasim"), geane_tree("koalasim") {
}

KoaSeperateDataReader::~KoaSeperateDataReader() {
}

unsigned int KoaSeperateDataReader::getEntries() const {

	if (MC_tree.GetEntries() > 0
			&& MC_tree.GetEntries() < geane_tree.GetEntries())
		return MC_tree.GetEntries();
	return geane_tree.GetEntries();
}

void KoaSeperateDataReader::initDataStream() {
	for (unsigned int i = 0; i < file_paths.size(); i++) {
		MC_tree.Add(file_paths[i] + "/Koala_MC*.root");
		track_tree.Add(file_paths[i] + "/Koala_Track*.root");
		geane_tree.Add(file_paths[i] + "/Koala_comp*.root");
	}

	MC_tree.SetBranchStatus("*", 0);
	track_tree.SetBranchStatus("*", 0);
	geane_tree.SetBranchStatus("*", 0);

	//--- MC info --------------------------------------------------------------------
	true_tracks = new TClonesArray("KoaMCTrack");
	MC_tree.SetBranchAddress("MCTrack", &true_tracks); //True Track to compare
	MC_tree.SetBranchStatus("MCTrack*", 1);

	true_points = new TClonesArray("PndSdsMCPoint");
	MC_tree.SetBranchAddress("LMDPoint", &true_points); //True Points to compare
	MC_tree.SetBranchStatus("LMDPoint*", 1);
	//--------------------------------------------------------------------------------

	//--- Track info -----------------------------------------------------------------
	lmd_tracks = new TClonesArray("PndTrack");
	track_tree.SetBranchAddress("LMDPndTrack", &lmd_tracks); //Reco Track to compare (at lumi monitor system, so not backtracked)
	track_tree.SetBranchStatus("LMDPndTrack*", 1);
	//--------------------------------------------------------------------------------

	//--- Geane info -----------------------------------------------------------------
	geane_tracks = new TClonesArray("PndTrack");
	geane_tree.SetBranchAddress("LMDCompa", &geane_tracks); //Tracks with parabolic parametrisation
	geane_tree.SetBranchStatus("LMDCompa*", 1); //Tracks with parabolic parametrisation

	mc_entries = MC_tree.GetEntries();
	num_entries = mc_entries;
	lmd_track_entries = track_tree.GetEntries();
	if (lmd_track_entries < mc_entries && lmd_track_entries > 0)
		num_entries = lmd_track_entries;
	else if (geane_tree.GetEntries() < num_entries
			&& geane_tree.GetEntries() > 0)
		num_entries = geane_tree.GetEntries();
}

void KoaSeperateDataReader::clearDataStream() {
}

std::vector<Lmd::Data::TrackPairInfo> KoaSeperateDataReader::getEntry(unsigned int i) {
	if (mc_entries > 0)
		MC_tree.GetEntry(i);
	if (lmd_track_entries > 0)
		track_tree.GetEntry(i);
	geane_tree.GetEntry(i);

	Lmd::Data::TrackPairInfo track_info;

	track_info.IsReconstructedAtIP = false;
	track_info.IsReconstructedAtLmd = false;

	for (int ik = 0; ik < true_points->GetEntriesFast(); ik++) {
		PndSdsMCPoint *lmd_point = (PndSdsMCPoint*) true_points->At(ik);
		TVector3 MomMC;
		TVector3 Pos(lmd_point->GetPosition());
		lmd_point->Momentum(MomMC);
		MomMC.RotateY(-0.040);

		if (-2212
				== ((KoaMCTrack*) true_tracks->At(lmd_point->GetTrackID()))->GetPdgCode()) {
			track_info.MCLMD.Position = {Pos.X(), Pos.Y(), Pos.Z()};
			track_info.MCLMD.Momentum = {MomMC.X(), MomMC.Y(), MomMC.Z()};
		}
	}
	for (int ik = 0; ik < true_tracks->GetEntries(); ik++) {
		KoaMCTrack *mctrk = (KoaMCTrack*) true_tracks->At(ik);
		TLorentzVector lv_mc = mctrk->Get4Momentum();
		Int_t mcID = mctrk->GetPdgCode();
		TVector3 MomMC_all = mctrk->GetMomentum();
		TVector3 Pos(mctrk->GetStartVertex());

		if (mcID == -2212 && mctrk->GetMotherId() == -1) {
			track_info.MCLMD.Position = {Pos.X(), Pos.Y(), Pos.Z()};
			track_info.MCLMD.Momentum = {MomMC_all.X(), MomMC_all.Y(), MomMC_all.Z()};
		}
	}

	for (Int_t iN = 0; iN < lmd_tracks->GetEntriesFast(); iN++) {
		PndTrack *track = (PndTrack*) lmd_tracks->At(iN);
		FairTrackParP trackpar = track->GetParamFirst();
	/*	double HitX = trackpar.GetX();
		double HitY = trackpar.GetY();
		double HitZ = trackpar.GetZ();
	        double PX = trackpar.GetPx();
		double PY = trackpar.GetPy();
		double PZ = trackpar.GetPz();
		

                double a = (PX * HitX + PY * HitY + PZ * HitZ)/(PX * PX + PY * PY + PZ * PZ);
	        double Xo = HitX - (PX * a);
		double Yo = HitY - (PY * a);
		double Zo = HitZ - (PZ * a);

			

		track_info.IsReconstructedAtLmd = true;
		track_info.RecoLMD.Position = {Xo, Yo,
				Zo};
		track_info.RecoLMD.Momentum = {pX, PY, PZ};
	}*/
		 TVector3 MomRec = trackpar.GetMomentum();

		                 track_info.IsReconstructedAtLmd = true;
				                 track_info.RecoLMD.Position = {trackpar.GetOrigin().X(), trackpar.GetOrigin().Y(),
							                                 trackpar.GetOrigin().Z()};
						                 track_info.RecoLMD.Momentum = {MomRec.X(), MomRec.Phi(), MomRec.Z()};
								         }

	// loop over geane tracks
	for (Int_t iN = 0; iN < geane_tracks->GetEntries(); iN++) {
		///-- Read info about GEANE(reconstructed) tracks--------------------------
		PndTrack *fResi = (PndTrack*) geane_tracks->At(iN);
		FairTrackParP fRes = fResi->GetParamFirst();			
	        TVector3 MomRec = fRes.GetMomentum();

		double HitX = fRes.GetX();
	        double HitY = fRes.GetY();	
		double HitZ = fRes.GetZ();
		double PX = fRes.GetPx();
		double PY = fRes.GetPy();
		double PZ = fRes.GetPz();


	
		double a = (PX * HitX + PY * HitY + PZ * HitZ)/(PX * PX + PY * PY + PZ * PZ);
															                double Xo = HitX - (PX * a);
																	double Yo = HitY - (PY * a);
																	double Zo = HitZ - (PZ * a);


			track_info.IsReconstructedAtIP = true;
			track_info.RecoIP.Position = {Xo, Yo, Zo};
			track_info.RecoIP.Momentum = {MomRec.X(), MomRec.Phi(), MomRec.Z()};
		
	}
	return {track_info};
}

