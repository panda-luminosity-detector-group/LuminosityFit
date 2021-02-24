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
class PndLmdMapData;
namespace Lmd {
    namespace Data {
      class TrackPairInfo;
  }
}


class TDatabasePDG;
class PndLmdDim;
class TClonesArray;

class PndLmdDataReader {
private:
  std::vector<PndLmdMapData*> registered_map_data;
  std::vector<PndLmdHistogramData*> registered_data;
  std::vector<PndLmdAcceptance*> registered_acceptances;

  void clearRegisters();

  std::vector<PndLmdAbstractData*> combineAllRegisteredDataObjects();

  void removeFinished(std::vector<PndLmdAbstractData*> &lmd_vec,
      int num_events);
  int getNextMinEventIndex(std::vector<PndLmdAbstractData*> &lmd_vec);

  double getSingleTrackParameterValue(const Lmd::Data::TrackPairInfo &track_info,
      const LumiFit::LmdDimension &lmd_dim) const;
  double getTrackParameterValue(const Lmd::Data::TrackPairInfo &track_info,
      const LumiFit::LmdDimension &lmd_dim) const;

  bool wasReconstructed(const Lmd::Data::TrackPairInfo &track_info) const;
  bool isGoodTrack(const Lmd::Data::TrackPairInfo &track_info) const;
  bool skipDataObject(const PndLmdAbstractData* data,
      const Lmd::Data::TrackPairInfo &track_info) const;
  bool successfullyPassedFilters(const PndLmdAbstractData* data,
      const Lmd::Data::TrackPairInfo &track_info) const;

  void fillData(const Lmd::Data::TrackPairInfo &track_pars);

  void cleanup();

  virtual unsigned int getEntries() const =0;
  virtual void initDataStream() =0;
  virtual void clearDataStream() =0;

  virtual std::vector<Lmd::Data::TrackPairInfo> getEntry(unsigned int i) =0;

  TLorentzVector beam;

protected:
  TDatabasePDG *pdg;

  std::vector<TString> file_paths;

public:
  PndLmdDataReader();
  virtual ~PndLmdDataReader();

  void setBeam(double lab_momentum);

  void addFilePath(TString file_path);
  void addFileList(const std::string &filelist);

  void registerMapData(std::vector<PndLmdMapData> &data_vec);
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
