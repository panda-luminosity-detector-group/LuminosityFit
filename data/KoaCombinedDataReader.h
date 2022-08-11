#ifndef KOACOMBINEDDATAREADER_H_
#define KOACOMBINEDDATAREADER_H_

#include "PndLmdDataReader.h"
#include "data/TrackData.h"

#include "TChain.h"
#include "TClonesArray.h"

class KoaCombinedDataReader : public PndLmdDataReader {
private:
  double rel_momentum_deviation_threshold;

  TChain data_tree;

  TClonesArray *comp_array;
  TClonesArray *koa_array;
  TClonesArray *MCtrack_array;
  TClonesArray *MCpoint_array;
  TClonesArray filtered_track_array;

  unsigned int getEntries() const;
  void initDataStream();
  void clearDataStream();

  std::vector<Lmd::Data::TrackPairInfo> getEntry(unsigned int i);

public:
  KoaCombinedDataReader();
  virtual ~KoaCombinedDataReader();
};

#endif /* KOACOMBINEDDATAREADER_H_ */
