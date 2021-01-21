#include "PndLmdCombinedDataReader.h"
#include "PndLmdTrackQ.h"

PndLmdCombinedDataReader::PndLmdCombinedDataReader() :
		rel_momentum_deviation_threshold(0.0003), data_tree("pndsim"), filtered_track_array(
				"PndLmdTrackQ", 100) {
	track_array = 0;
}

PndLmdCombinedDataReader::~PndLmdCombinedDataReader() {
}

unsigned int PndLmdCombinedDataReader::getEntries() const {
	return data_tree.GetEntries();
}

void PndLmdCombinedDataReader::initDataStream() {
	for (unsigned int i = 0; i < file_paths.size(); i++)
		data_tree.Add(file_paths[i]);

	track_array = new TClonesArray("PndLmdTrackQ");
	data_tree.SetBranchAddress("LMDTrackQ", &track_array);
	data_tree.SetBranchStatus("LMDTrackQ*", 1);
}

void PndLmdCombinedDataReader::clearDataStream() {
}

std::vector<Lmd::Data::TrackPairInfo> PndLmdCombinedDataReader::getEntry(unsigned int i) {
	data_tree.GetEntry(i);
	filtered_track_array.Clear();
	unsigned int good_track_counter = 0;

	std::vector<Lmd::Data::TrackPairInfo> track_pairs;

	//loop over track array
	for (Int_t iN = 0; iN < track_array->GetEntries(); iN++) {
		PndLmdTrackQ *track = (PndLmdTrackQ*) track_array->At(iN);
		TVector3 temp_vec;

		Lmd::Data::TrackPairInfo track_info;
		if(0 == track->GetTrkRecStatus()) {
			track_info.IsReconstructedAtLmd = true;
			track_info.IsReconstructedAtIP = true;
		}
		if(-10 == track->GetTrkRecStatus())
			track_info.IsReconstructedAtLmd = true;
		if(0 <= track->GetSecondary())
			track_info.IsSecondary = true;

		if(track_info.IsReconstructedAtIP) {
			track->GetIPpoint(temp_vec);
			track_info.RecoIP.Position = {temp_vec.X(), temp_vec.Y(), temp_vec.Z()};
			TVector3 mom;
			mom.SetMagThetaPhi(track->GetIPmom(), track->GetIPtheta(), track->GetIPphi());
			track_info.RecoIP.Momentum = {mom.X(), mom.Y(), mom.Z()};
		}

		if(track_info.IsReconstructedAtLmd) {
			track->GetLMDpoint(temp_vec);
			track_info.RecoLMD.Position = {temp_vec.X(), temp_vec.Y(), temp_vec.Z()};
			TVector3 mom;
			mom.SetMagThetaPhi(track->GetLMDphi(), track->GetIPtheta(), track->GetIPphi());
			track_info.RecoLMD.Momentum = {mom.X(), mom.Y(), mom.Z()};
		}

		track->GetMCpoint(temp_vec);
		track_info.MCIP.Position = {temp_vec.X(), temp_vec.Y(), temp_vec.Z()};
		TVector3 mom;
		mom.SetMagThetaPhi(track->GetMCmom(), track->GetMCtheta(), track->GetMCphi());
		track_info.MCIP.Momentum = {mom.X(), mom.Y(), mom.Z()};

		track->GetMCpointLMD(temp_vec);
		track_info.MCLMD.Position = {temp_vec.X(), temp_vec.Y(), temp_vec.Z()};
		mom.SetMagThetaPhi(track->GetMCmomLMD(), track->GetMCthetaLMD(), track->GetMCphiLMD());
		track_info.MCLMD.Momentum = {mom.X(), mom.Y(), mom.Z()};
		
		track_pairs.push_back(track_info);
	}

	return track_pairs;
}
