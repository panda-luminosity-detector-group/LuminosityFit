/*
 * ROOTDataHelper.h
 *
 *  Created on: Jun 15, 2013
 *      Author: steve
 */

#ifndef ROOTDATAHELPER_H_
#define ROOTDATAHELPER_H_

#include "SharedPtr.h"

class TH1D;
class TH2D;
class TGraphErrors;

class Data;

class ROOTDataHelper {
public:
	ROOTDataHelper();
	virtual ~ROOTDataHelper();

	void fillBinnedData(shared_ptr<Data> data, const TH1D* hist_1d) const;
	void fillBinnedData(shared_ptr<Data> data, const TH2D* hist_2d) const;

	void fillBinnedData(shared_ptr<Data> data, const TGraphErrors* graph_1d) const;
};

#endif /* ROOTDATAHELPER_H_ */
