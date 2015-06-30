/*
 * PndLmdCombinedDataReader.h
 *
 *  Created on: Aug 26, 2013
 *      Author: steve
 */

#ifndef PNDLMDCOMBINEDDATAREADER_H_
#define PNDLMDCOMBINEDDATAREADER_H_

#include "PndLmdDataReader.h"

#include "TChain.h"
#include "TClonesArray.h"

class PndLmdCombinedDataReader: public PndLmdDataReader {
  private:
	double rel_momentum_deviation_threshold;

    TChain data_tree;

    TClonesArray* track_array;
    TClonesArray filtered_track_array;


    unsigned int getEntries() const;
    void initDataStream();
    void clearDataStream();

    TClonesArray* getEntry(unsigned int i);
  public:
    PndLmdCombinedDataReader();
    virtual ~PndLmdCombinedDataReader();
};

#endif /* PNDLMDCOMBINEDDATAREADER_H_ */
