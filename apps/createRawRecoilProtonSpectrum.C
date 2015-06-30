/*
 * createRawRecoilProtonSpectrum.C
 *
 *  Created on: Sep 17, 2014
 *      Author: steve
 */

#include <sstream>
#include <iostream>
#include <fstream>

#include "TFile.h"
#include "TChain.h"
#include "TClonesArray.h"
#include "TParticle.h"
#include "TMath.h"
#include "TH2D.h"

void createRawRecoilProtonSpectrum(std::string data_path,
		bool is_filelist_path = false, int index = -1, int num_events = -1) {
	TChain tree("data");

	if (is_filelist_path) {
		std::stringstream ss;
		ss << data_path << "/filelist_" << index << ".txt";
		std::cout << "Opening " << ss.str() << std::endl;
		std::ifstream infile(ss.str().c_str());
		std::string line;
		while (std::getline(infile, line)) {
			std::istringstream iss(line);

			std::cout << "Adding " << line.c_str() << std::endl;
			tree.Add(line.c_str());
		}
	} else {
		std::stringstream ss;
		if (index != -1) {
			ss << data_path << "/*_" << index << ".root";
			tree.Add(ss.str().c_str());
		} else {
			ss << data_path << "/*.root";
			tree.Add(ss.str().c_str());
		}
	}

	TClonesArray* fEvt = new TClonesArray("TParticle", 100);
	tree.SetBranchAddress("Particles", &fEvt);

	std::cout << "tree has " << tree.GetEntries() << " entries" << std::endl;
	unsigned int num_events_to_process = tree.GetEntries();
	if (num_events != -1 && num_events < num_events_to_process)
		num_events_to_process = num_events;

	// create histograms from raw dpm files
	TH2D* proton_angle_vs_mom_hist = new TH2D("proton_angle_vs_mom", "", 100,
			0, 0.09, 100, 0, 0.165);

	std::cout << "Processing " << num_events_to_process << " events!"
			<< std::endl;

	for (unsigned int i = 0; i < num_events_to_process; i++) {
		tree.GetEntry(i);
		TLorentzVector outgoing_pbar;
		TVector3 pbar_vect(outgoing_pbar.Vect());

		TLorentzVector outgoing;
		TVector3 proton_vect(outgoing.Vect());

		for (unsigned int np = 0; np < fEvt->GetEntries(); np++) {
			TParticle *particle = (TParticle*) fEvt->At(np);

			if (particle->GetPdgCode() == -2212) {
				particle->Momentum(outgoing_pbar);
				pbar_vect = outgoing_pbar.Vect();
			}

			else if (particle->GetPdgCode() == 2212) {
				particle->Momentum(outgoing);
				proton_vect = outgoing.Vect();
			}
		}

		if (pbar_vect.Theta() < 0.01) {
			proton_angle_vs_mom_hist->Fill(TMath::PiOver2() - proton_vect.Theta(),
					proton_vect.Mag());
		}
	}

	TFile *f = new TFile("recoil_proton_spectrum.root", "RECREATE");
	proton_angle_vs_mom_hist->Write();
	f->Close();
}

