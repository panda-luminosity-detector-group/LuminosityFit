#ifndef PNDLMDHISTOGRAMDATA_H_
#define PNDLMDHISTOGRAMDATA_H_

#include "PndLmdAbstractData.h"
#include "fit/PndLmdFitStorage.h"

#include "TH1D.h" // these includes I need for the dictionary generation
#include "TH2D.h"

class PndLmdHistogramData: public PndLmdAbstractData  {
	/** ROOT 1D histogram as the container of the data */
	TH1D* hist_1d;
	/** ROOT 2D histogram as the container of the data */
	TH2D* hist_2d;

	PndLmdFitStorage fit_storage;

	void init1DData();
	void init2DData();

public:
	PndLmdHistogramData();
	PndLmdHistogramData(const PndLmdHistogramData &lmd_hist_data_);
	virtual ~PndLmdHistogramData();

	TH1D* get1DHistogram() const;
	TH2D* get2DHistogram() const;

	void add(const PndLmdAbstractData &lmd_abs_data_addition);

	// histogram filling methods
	virtual void addData(double primary_value, double secondary_value = 0);

	const std::map<PndLmdFitOptions, std::vector<ModelFitResult>>& getFitResults() const;
	std::vector<ModelFitResult> getFitResults(const PndLmdFitOptions &fit_options) const;
	void addFitResult(const PndLmdFitOptions &fit_options, const ModelFitResult &fit_result_);

	PndLmdHistogramData& operator=(const PndLmdHistogramData &lmd_hist_data);

	ClassDef(PndLmdHistogramData, 3);
};

#endif /* PNDLMDHISTOGRAMDATA_H_ */
