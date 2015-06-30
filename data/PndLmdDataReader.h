/*
 * PndLmdDataReader.h
 *
 *  Created on: Aug 24, 2013
 *      Author: steve
 */

#ifndef PNDLMDDATAREADER_H_
#define PNDLMDDATAREADER_H_

#include "LumiFitStructs.h"

#include <vector>

#include "TString.h"
#include "TLorentzVector.h"

class PndLmdAbstractData;
class PndLmdHistogramData;
class PndLmdAngularData;
class PndLmdAcceptance;
class PndLmdResolution;

class TDatabasePDG;
class PndLmdDim;
class PndLmdTrackQ;
class TClonesArray;

class PndLmdDataReader {
private:
	std::vector<PndLmdHistogramData*> registered_data;
	std::vector<PndLmdAcceptance*> registered_acceptances;

	void clearRegisters();

	std::vector<PndLmdAbstractData*> combineAllRegisteredDataObjects();

	void removeFinished(std::vector<PndLmdAbstractData*> &lmd_vec,
			int num_events);
	int getNextMinEventIndex(std::vector<PndLmdAbstractData*> &lmd_vec);

	double getSingleTrackParameterValue(PndLmdTrackQ &track_pars,
			const LumiFit::LmdDimension &lmd_dim) const;
	double getTrackParameterValue(PndLmdTrackQ &track_pars,
			const LumiFit::LmdDimension &lmd_dim) const;

	bool wasReconstructed(PndLmdTrackQ &track_pars) const;
	bool skipDataObject(const PndLmdAbstractData* data,
			PndLmdTrackQ &track_pars) const;
	bool successfullyPassedFilters(const PndLmdAbstractData* data,
			PndLmdTrackQ &track_pars) const;

	void fillData(PndLmdTrackQ *track_pars);

	void cleanup();

	virtual unsigned int getEntries() const =0;
	virtual void initDataStream() =0;
	virtual void clearDataStream() =0;

	virtual TClonesArray* getEntry(unsigned int i) =0;

	TLorentzVector beam;

protected:
	TDatabasePDG *pdg;

	std::vector<TString> file_paths;

public:
	PndLmdDataReader();
	virtual ~PndLmdDataReader();

	void setBeam(double lab_momentum);

	void addFilePath(TString file_path);

	int registerData(PndLmdHistogramData* data);
	int registerData(std::vector<PndLmdAngularData> &data_vec);
	int registerData(std::vector<PndLmdHistogramData> &data_vec);

	int registerAcceptance(PndLmdAcceptance* acc);
	int registerAcceptances(std::vector<PndLmdAcceptance> &acc_vec);

	int registerResolution(PndLmdResolution* res);
	int registerResolutions(std::vector<PndLmdResolution> &res_vec);

	void read();
};

#endif /* PNDLMDDATAREADER_H_ */
