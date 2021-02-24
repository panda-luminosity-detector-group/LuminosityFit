#ifndef PNDLMDSEPERATEDATAREADER_H_
#define PNDLMDSEPERATEDATAREADER_H_

#include "PndLmdDataReader.h"
#include "data/TrackData.h"

#include "TChain.h"
#include "TClonesArray.h"

class PndLmdSeperateDataReader: public PndLmdDataReader {
  private:
	  unsigned int num_entries;
	  unsigned int mc_entries;
	  unsigned int lmd_track_entries;

    TChain MC_tree;
    TChain track_tree;
    TChain geane_tree;

    //--- MC info --------------------------------------------------------------------
    TClonesArray* true_tracks;
    TClonesArray* true_points;
    //--- Track info -----------------------------------------------------------------
    TClonesArray* lmd_tracks;
    //--- Geane info -----------------------------------------------------------------
    TClonesArray* geane_tracks;

    unsigned int getEntries() const;
    void initDataStream();
    void clearDataStream();

    std::vector<Lmd::Data::TrackPairInfo> getEntry(unsigned int i);
  public:
    PndLmdSeperateDataReader();
    virtual ~PndLmdSeperateDataReader();
};

#endif /* PNDLMDSEPERATEDATAREADER_H_ */
