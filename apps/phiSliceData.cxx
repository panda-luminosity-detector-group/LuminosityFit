#include "TChain.h"
#include "TClonesArray.h"
#include "TFile.h"
#include "PndLmdTrackQ.h"
#include "TMath.h"
#include "PndLmdDim.h"

#include <iostream>
#include <sstream>
#include <map>

unsigned int num_phi_slices = 10;
unsigned int start_phi = 0.0;

PndLmdDim *lmd_dim;

struct DataBundle {
	TFile *f;
	TTree *t;
	TClonesArray *track_array;

	std::pair<double, double> phi_range;

	DataBundle() {
	}

	DataBundle(int i, std::string dir) {
		int ilow = i - 1;
		if (i < 0) {
			ilow = i + 1;
			phi_range = std::make_pair(TMath::Pi() / (num_phi_slices / 2) * i,
					TMath::Pi() / (num_phi_slices / 2) * ilow);
		} else
			phi_range = std::make_pair(TMath::Pi() / (num_phi_slices / 2) * ilow,
					TMath::Pi() / (num_phi_slices / 2) * i);

		std::stringstream ss;
		ss << dir << "/Lumi_TrksQA_phi_slice_" << i << ".root";

		f = new TFile(ss.str().c_str(), "RECREATE");

		t = new TTree("cbmsim", "");
		track_array = new TClonesArray("PndLmdTrackQ");
		t->Branch("LMDTrackQ", &track_array);
	}

	void fillIfDataIsWithinRange(TClonesArray tracks) {
		TClonesArray &ca = *track_array;
		track_array->Clear();
		int track_counter = 0;
		for (Int_t iN = 0; iN < tracks.GetEntries(); iN++) {
			PndLmdTrackQ *track = (PndLmdTrackQ*) tracks.At(iN);
			//TVector3 v(1,1,1);
			//v.SetTheta(track->GetLMDtheta()); // keeping rho and phi
			//v.SetPhi(track->GetLMDphi());   // keeping rho and theta
			//v.SetMag(1.);  // keeping theta and phi 1, track->GetLMDphi(),
			bool write = true;
			if (track->GetTrkRecStatus() >= 0) {
				TVector3 v;
				track->GetMCpointLMD(v);
				TVector3 local = lmd_dim->Transform_global_to_lmd_local(v);

				double phi = atan2(local.Y(), local.X());
				if (phi < phi_range.first || phi > phi_range.second) {
					//write = false;
					track->SetTrkRecStatus(-3);
				}
			}
			if (write) {
				ca[track_counter++] = track;
			}
		}
		t->Fill();
	}

	void saveToFile() {
		f->cd();
		t->Write();
		f->Close();
	}
};

void phiSliceData(TString dir) {
	TChain chain("cbmsim");
	chain.Add(dir + "/Lumi_TrksQA_*0.root");

	TClonesArray *track_array = new TClonesArray("PndLmdTrackQ");
	chain.SetBranchAddress("LMDTrackQ", &track_array);
	chain.SetBranchStatus("LMDTrackQ*", 1);

	// open files for filtered data
	std::map<int, DataBundle> new_root_files;
	for (unsigned int i = 0; i < num_phi_slices / 2; i++) {
		new_root_files[-(i + 1)] = DataBundle(-(i + 1), dir.Data());
		new_root_files[(i + 1)] = DataBundle((i + 1), dir.Data());
	}

	std::map<int, DataBundle>::iterator it;

	std::cout << "entries in files: " << chain.GetEntries() << std::endl;
	for (unsigned int i = 0; i < chain.GetEntries(); i++) {
		chain.GetEntry(i);

		if (i % 10000 == 0)
			std::cout << i << std::endl;

		//loop over map
		for (it = new_root_files.begin(); it != new_root_files.end(); it++) {
			it->second.fillIfDataIsWithinRange(*track_array);
		}
	}

	for (it = new_root_files.begin(); it != new_root_files.end(); it++) {
		it->second.saveToFile();
	}
}

int main(int argc, char* argv[]) {
	lmd_dim = PndLmdDim::Instance();
	lmd_dim->Read_transformation_matrices();
	if (argc == 2) {
		phiSliceData(TString(argv[1]));
	}
}
