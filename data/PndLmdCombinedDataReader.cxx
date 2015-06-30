/*
 * PndLmdCombinedDataReader.cxx
 *
 *  Created on: Aug 26, 2013
 *      Author: steve
 */

#include "PndLmdCombinedDataReader.h"
#include "PndLmdTrackQ.h"

PndLmdCombinedDataReader::PndLmdCombinedDataReader() :
		rel_momentum_deviation_threshold(0.0003), data_tree("cbmsim"), filtered_track_array(
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

TClonesArray* PndLmdCombinedDataReader::getEntry(unsigned int i) {
	data_tree.GetEntry(i);
	filtered_track_array.Clear();
	unsigned int good_track_counter = 0;

	//loop over track array
	for (Int_t iN = 0; iN < track_array->GetEntries(); iN++) {
		PndLmdTrackQ *track = (PndLmdTrackQ*) track_array->At(iN);

		//if (-2212 == track->GetPDGcode()) {
		/*	if (track->GetTrkRecStatus() >= 0
			 && (track->GetIPmom() - track->GetMCmom())
			 / track->GetMCmom()
			 > rel_momentum_deviation_threshold)
			 track->SetTrkRecStatus(-3);*/
			filtered_track_array[good_track_counter++] = track;
		//}
	}

	return &filtered_track_array;
}
