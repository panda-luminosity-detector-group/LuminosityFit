// this macro checks the difference between the MC information of the generated pbars and the
#include "PndMCTrack.h"
#include "PndLmdTrackQ.h"

#include <iostream>

#include "TChain.h"
#include "TClonesArray.h"
#include "TH1D.h"
#include "TH1D.h"
#include "TParticle.h"
#include "TCanvas.h"

using std::cout;
using std::endl;

void checkMCInfoTrackMatching(TString path, TString gen_data) {
	TChain mc_tree("cbmsim");
	mc_tree.Add(path + "/Lumi_MC*.root");
	TClonesArray *mctrack_array = new TClonesArray("PndMCTrack");
	mc_tree.SetBranchAddress("MCTrack", &mctrack_array);
	mc_tree.SetBranchStatus("MCTrack*", 1);

	TChain qtrack_tree("cbmsim");
	qtrack_tree.Add(path + "/Lumi_TrksQA*.root");
	TClonesArray *qtrack_array = new TClonesArray("PndLmdTrackQ");
	qtrack_tree.SetBranchAddress("LMDTrackQ", &qtrack_array);
	qtrack_tree.SetBranchStatus("LMDTrackQ*", 1);

	unsigned int mc_events = mc_tree.GetEntries();
	unsigned int qmatch_events = qtrack_tree.GetEntries();

	cout << mc_events << " " << qmatch_events << endl;

	TH1D *mc_hist = new TH1D("mc", "mc", 400, 0.0, 0.02);
	TH1D *qtrack_hist = new TH1D("qtrack", "qtrack", 400, 0.0, 0.02);

	for (unsigned int i = 0; i < mc_events; i++) {
		mc_tree.GetEntry(i);
		qtrack_tree.GetEntry(i);

		// mc info
		for (unsigned int k = 0; k < mctrack_array->GetEntries(); k++) {
			PndMCTrack *mctrack = (PndMCTrack*) mctrack_array->At(k);
			if (mctrack->IsGeneratorCreated() && mctrack->GetPdgCode() == -2212) {
				mc_hist->Fill(mctrack->GetMomentum().Theta());
			}
		}

		// qtrack info
		//if (qtrack_array->GetEntries() == 2) {
		for (unsigned int k = 0; k < qtrack_array->GetEntries(); k++) {
			PndLmdTrackQ *qtrack = (PndLmdTrackQ*) qtrack_array->At(k);
			if (qtrack->GetPDGcode() == -2212) {
				qtrack_hist->Fill(qtrack->GetMCtheta());
			}
		}
		//}
	}


	// open the corresponding generator data

	TChain tree("data");
	tree.Add(gen_data + "/*.root");

	TClonesArray* fEvt = new TClonesArray("TParticle", 100);
	tree.SetBranchAddress("Particles", &fEvt);
	tree.SetBranchStatus("Particles*", 1);

	std::cout << "tree has " << tree.GetEntries() << " entries" << std::endl;

	unsigned int num_events_to_process = tree.GetEntries();

	TH1D *histtheta = new TH1D("hist_dpm_theta", "", 400, 0.0, 0.02);

	for (unsigned int i = 0; i < num_events_to_process; i++) {
		tree.GetEntry(i);
		for (unsigned int np = 0; np < fEvt->GetEntries(); np++) {
			TParticle *particle = (TParticle*) fEvt->At(np);
			if (particle->GetPdgCode() == -2212) {
				TLorentzVector outgoing;
				particle->Momentum(outgoing);
				histtheta->Fill(outgoing.Theta());
			}
		}
	}

	TCanvas *c = new TCanvas();
	c->Divide(2,1);
	TH1D *diff = new TH1D(*mc_hist);
	diff->Add(qtrack_hist, -1.0);

	TH1D *dpm_mc_diff = new TH1D(*histtheta);
	dpm_mc_diff->Add(mc_hist, -1.0);

	c->cd(1);
	diff->Draw();
	c->cd(2);
	dpm_mc_diff->Draw();

	c->SaveAs("test.pdf");
}

int main(int argc, char* argv[]) {
	if (argc == 3) {
		checkMCInfoTrackMatching(argv[1], argv[2]);
	}

	return 0;
}

