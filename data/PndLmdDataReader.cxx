#include "PndLmdDataReader.h"
#include "PndLmdAbstractData.h"
#include "PndLmdAcceptance.h"
#include "PndLmdAngularData.h"
#include "PndLmdHistogramData.h"
#include "PndLmdMapData.h"

#include <fstream>
#include <iostream>
#include <set>

#include "boost/progress.hpp"

#include "TrackData.h"

#include "TChain.h"
#include "TClonesArray.h"
#include "TDatabasePDG.h"
#include "TMath.h"

PndLmdDataReader::PndLmdDataReader() : beam(0.0, 0.0, 0.0, 0.0) {
  pdg = TDatabasePDG::Instance();
}

PndLmdDataReader::~PndLmdDataReader() {}

void PndLmdDataReader::registerMapData(std::vector<PndLmdMapData> &data_vec) {
  for (unsigned int i = 0; i < data_vec.size(); i++) {
    registered_map_data.push_back(&data_vec[i]);
  }
}

int PndLmdDataReader::registerData(PndLmdHistogramData *data) {
  registered_data.push_back(data);
  return 0;
}

int PndLmdDataReader::registerData(std::vector<PndLmdAngularData> &data_vec) {
  for (unsigned int i = 0; i < data_vec.size(); i++) {
    registered_data.push_back(&data_vec[i]);
  }
  return 0;
}

int PndLmdDataReader::registerData(std::vector<PndLmdHistogramData> &data_vec) {
  for (unsigned int i = 0; i < data_vec.size(); i++) {
    registered_data.push_back(&data_vec[i]);
  }
  return 0;
}

int PndLmdDataReader::registerAcceptance(PndLmdAcceptance *acc) {
  registered_acceptances.push_back(acc);
  return 0;
}

int PndLmdDataReader::registerAcceptances(
    std::vector<PndLmdAcceptance> &acc_vec) {
  for (unsigned int i = 0; i < acc_vec.size(); i++) {
    registered_acceptances.push_back(&acc_vec[i]);
  }
  return 0;
}

void PndLmdDataReader::clearRegisters() {
  registered_data.clear();
  registered_acceptances.clear();
}

void PndLmdDataReader::removeFinished(
    std::vector<PndLmdAbstractData *> &lmd_vec, int current_event_index) {
  std::vector<PndLmdAbstractData *>::iterator it = lmd_vec.begin();
  while (it != lmd_vec.end()) {
    if (current_event_index == (*it)->getNumEvents()) {
      (*it)->setNumEvents(current_event_index);
      lmd_vec.erase(it);
      it--;
    }
    it++;
  }
}

int PndLmdDataReader::getNextMinEventIndex(
    std::vector<PndLmdAbstractData *> &lmd_vec) {
  int next_min_event_index = -1;
  for (unsigned int i = 0; i < lmd_vec.size(); i++) {
    if (lmd_vec[i]->getNumEvents() < next_min_event_index)
      next_min_event_index = lmd_vec[i]->getNumEvents();
    else if (next_min_event_index == -1) {
      next_min_event_index = lmd_vec[i]->getNumEvents();
    }
  }
  return next_min_event_index;
}

void PndLmdDataReader::addFilePath(TString file_path) {
  std::cout << "adding file path: " << file_path << std::endl;
  file_paths.push_back(file_path);
}

void PndLmdDataReader::addFileList(const std::string &filelist) {
  // scan which data reader has to create this object
  std::ifstream input(filelist.c_str());
  std::string line;

  while (std::getline(input, line)) {
    std::cout << line << std::endl;
    addFilePath(line);
  }
}

std::vector<PndLmdAbstractData *>
PndLmdDataReader::combineAllRegisteredDataObjects() {
  std::vector<PndLmdAbstractData *> lmd_vec;
  std::cout << registered_data.size() << " data objects are registered!"
            << std::endl;
  for (unsigned int i = 0; i < registered_data.size(); i++) {
    lmd_vec.push_back(registered_data[i]);
  }
  std::cout << registered_acceptances.size()
            << " acceptance objects are registered!" << std::endl;
  for (unsigned int i = 0; i < registered_acceptances.size(); i++) {
    lmd_vec.push_back(registered_acceptances[i]);
  }
  std::cout << registered_map_data.size() << " map data objects are registered!"
            << std::endl;
  for (unsigned int i = 0; i < registered_map_data.size(); i++) {
    lmd_vec.push_back(registered_map_data[i]);
  }
  return lmd_vec;
}

void PndLmdDataReader::cleanup() {
  clearDataStream();
  clearRegisters();
}

void PndLmdDataReader::setBeam(double lab_momentum) {
  beam = TLorentzVector(
      0, 0, lab_momentum,
      sqrt(pow(lab_momentum, 2.0) + pow(pdg->GetParticle(-2212)->Mass(), 2.0)));
}

void PndLmdDataReader::read() {
  std::vector<PndLmdAbstractData *> lmd_vec = combineAllRegisteredDataObjects();

  if (0 == lmd_vec.size()) {
    std::cout << "No data or acceptance objects were registered, hence no data "
                 "can be filled."
                 " Please register objects via the register methods of this "
                 "helper class"
              << std::endl;
    return;
  }

  initDataStream();

  int temp_num_events = getEntries();
  int num_events = 0;

  // all data objects initialized with -1 as number of events
  // should be filled with all found data -> adjust that now
  for (unsigned int i = 0; i < lmd_vec.size(); i++) {
    if (0 == lmd_vec[i]->getNumEvents()) {
      lmd_vec[i]->setNumEvents(temp_num_events);
    }
  }

  // get maximum event number for all registered data objects
  for (unsigned int i = 0; i < lmd_vec.size(); i++) {
    if (num_events < lmd_vec[i]->getNumEvents())
      num_events = lmd_vec[i]->getNumEvents();
    else if (lmd_vec[i]->getNumEvents() == -1) {
      num_events = temp_num_events;
      break;
    }
    if (num_events > temp_num_events) {
      num_events = temp_num_events;
      break;
    }
  }
  // now run over all registered data objects once again and adjust
  // the number of events accordingly
  for (unsigned int i = 0; i < lmd_vec.size(); i++) {
    if (num_events < lmd_vec[i]->getNumEvents())
      lmd_vec[i]->setNumEvents(num_events);
    else if (lmd_vec[i]->getNumEvents() == -1) {
      lmd_vec[i]->setNumEvents(num_events);
    }
  }
  int min_num_events = getNextMinEventIndex(lmd_vec);

  std::cout << "Processing " << num_events << " events" << std::endl;

  boost::progress_display show_progress(num_events);

  for (Int_t j = 0; j < num_events; j++) {
    // this check is to remove the data registered data objects for which
    // the event count has reached the specified value
    if (j == min_num_events) {
      // remove the data objects that hit their specified event count
      removeFinished(lmd_vec, j);
      // adjust the minimum number of events to the remaining data objects
      min_num_events = getNextMinEventIndex(lmd_vec);
    }

    std::vector<Lmd::Data::TrackPairInfo> track_pair_infos = getEntry(j);
    for (auto const &trk_pair_info : track_pair_infos) {
      fillData(trk_pair_info);
    }
    ++show_progress;
  }

  cleanup();
}

void PndLmdDataReader::fillData(const Lmd::Data::TrackPairInfo &track_info) {
  if (isGoodTrack(track_info)) {
    std::vector<double> data(4);
    auto const &mom_mc = track_info.MCIP.Momentum;
    auto const &mom_rec = track_info.RecoIP.Momentum;
    data[0] = mom_rec[0] / mom_rec[2];
    data[1] = mom_rec[1] / mom_rec[2];
    data[2] = mom_mc[0] / mom_mc[2];
    data[3] = mom_mc[1] / mom_mc[2];

    for (unsigned int i = 0; i < registered_map_data.size(); ++i) {
      registered_map_data[i]->addData(data);
    }
  }

  for (unsigned int i = 0; i < registered_data.size(); i++) {
    if (!skipDataObject(registered_data[i], track_info)) {
      if (successfullyPassedFilters(registered_data[i], track_info)) {
        if (registered_data[i]->getSecondaryDimension().is_active) {
          registered_data[i]->addData(
              getTrackParameterValue(track_info,
                                     registered_data[i]->getPrimaryDimension()),
              getTrackParameterValue(
                  track_info, registered_data[i]->getSecondaryDimension()));
        } else {
          registered_data[i]->addData(getTrackParameterValue(
              track_info, registered_data[i]->getPrimaryDimension()));
        }
      }
    }
  }
  for (unsigned int i = 0; i < registered_acceptances.size(); i++) {
    bool track_accepted = wasReconstructed(track_info);
    if (track_info.IsSecondary) {
      if (!track_accepted)
        continue;
    }
    if (track_accepted) {
      double pX = track_info.MCIP.Momentum[0];
      double pY = track_info.MCIP.Momentum[1];
      double pZ = track_info.MCIP.Momentum[2];
      double thetaX = pX / pZ;
      double thetaY = pY / pZ;

      double thetaPlane = sqrt(thetaX * thetaX + thetaY * thetaY);

      // FIXME: this doesn't belong here, but more appropriately in the track
      // reconstruction that is in PandaRoot however, not Lumi Fit
      // if (thetaPlane < 0.003 || thetaPlane > 0.0105) {
      if (thetaPlane < 0.003) {
        track_accepted = false;
        std::cout << "skipping track with thetaPlane = " << thetaPlane
                  << std::endl;
      }
    }

    // skip tracks that do not pass the filters
    if (successfullyPassedFilters(registered_acceptances[i], track_info)) {
      if (registered_acceptances[i]->getSecondaryDimension().is_active) {
        registered_acceptances[i]->addData(
            track_accepted,
            getTrackParameterValue(
                track_info, registered_acceptances[i]->getPrimaryDimension()),
            getTrackParameterValue(
                track_info,
                registered_acceptances[i]->getSecondaryDimension()));
      } else {
        registered_acceptances[i]->addData(
            track_accepted,
            getTrackParameterValue(
                track_info, registered_acceptances[i]->getPrimaryDimension()));
      }
    }
  }
}

bool PndLmdDataReader::isGoodTrack(
    const Lmd::Data::TrackPairInfo &track_info) const {
  return (track_info.IsReconstructedAtIP);
}

bool PndLmdDataReader::wasReconstructed(
    const Lmd::Data::TrackPairInfo &track_info) const {
  return (track_info.IsReconstructedAtIP);
}

bool PndLmdDataReader::skipDataObject(
    const PndLmdAbstractData *data,
    const Lmd::Data::TrackPairInfo &track_info) const {
  // if it was not reconstructed and this information is required
  if (!wasReconstructed(track_info)) {
    if (LumiFit::RECO ==
        data->getPrimaryDimension().dimension_options.track_type) {
      return true;
    } else if (LumiFit::MC_ACC ==
                   data->getPrimaryDimension().dimension_options.track_type ||
               LumiFit::DIFF_RECO_MC ==
                   data->getPrimaryDimension().dimension_options.track_type) {
      return true;
    }
    if (data->getSecondaryDimension().is_active) {
      if (LumiFit::RECO ==
          data->getSecondaryDimension().dimension_options.track_type)
        return true;
      else if (LumiFit::MC_ACC ==
                   data->getSecondaryDimension().dimension_options.track_type ||
               LumiFit::DIFF_RECO_MC ==
                   data->getSecondaryDimension().dimension_options.track_type) {
        return true;
      }
    }
  }
  if (LumiFit::MC == data->getPrimaryDimension().dimension_options.track_type) {
    if (track_info.IsSecondary)
      return true;
  }
  return false;
}

bool PndLmdDataReader::successfullyPassedFilters(
    const PndLmdAbstractData *data,
    const Lmd::Data::TrackPairInfo &track_info) const {
  // if it fails to pass a filter
  const std::set<LumiFit::LmdDimension> &selection_dimensions(
      data->getSelectorSet());
  std::set<LumiFit::LmdDimension>::iterator selection_dimension_iterator =
      selection_dimensions.begin();
  while (selection_dimension_iterator != selection_dimensions.end()) {
    if (false ==
        selection_dimension_iterator->dimension_range.isDataWithinRange(
            getTrackParameterValue(track_info,
                                   *selection_dimension_iterator))) {
      return false;
    }
    ++selection_dimension_iterator;
  }
  return true;
}

double PndLmdDataReader::getTrackParameterValue(
    const Lmd::Data::TrackPairInfo &track_info,
    const LumiFit::LmdDimension &lmd_dim) const {
  TVector3 pos(0.0, 0.0, 0.0);
  TVector3 mom(0.0, 0.0, 1.0);
  double theta(0.0);
  double phi(0.0);
  double theta_x(0.0);
  double theta_y(0.0);

  if (lmd_dim.dimension_options.dimension_type == LumiFit::PARTICLE_ID) {
    return 1.0 * track_info.PDGCode;
  }

  if (lmd_dim.dimension_options.dimension_type == LumiFit::SECONDARY) {
    return 1.0 * track_info.IsSecondary;
  }

  if (lmd_dim.dimension_options.track_type == LumiFit::MC ||
      lmd_dim.dimension_options.track_type == LumiFit::MC_ACC) {
    if (lmd_dim.dimension_options.track_param_type == LumiFit::IP) {
      auto mc_pos = track_info.MCIP.Position;
      pos.SetXYZ(mc_pos[0], mc_pos[1], mc_pos[2]);
      auto mc_mom = track_info.MCIP.Momentum;
      mom.SetXYZ(mc_mom[0], mc_mom[1], mc_mom[2]);
    } else if (lmd_dim.dimension_options.track_param_type == LumiFit::LMD) {
      auto mc_pos = track_info.MCLMD.Position;
      pos.SetXYZ(mc_pos[0], mc_pos[1], mc_pos[2]);
      auto mc_mom = track_info.MCLMD.Momentum;
      mom.SetXYZ(mc_mom[0], mc_mom[1], mc_mom[2]);
    }
    theta = mom.Theta();
    phi = mom.Phi();
    theta_x = mom.X() / mom.Z();
    theta_y = mom.Y() / mom.Z();
  } else if (lmd_dim.dimension_options.track_type == LumiFit::RECO) {
    if (lmd_dim.dimension_options.track_param_type == LumiFit::IP) {
      auto reco_pos = track_info.RecoIP.Position;
      pos.SetXYZ(reco_pos[0], reco_pos[1], reco_pos[2]);
      auto reco_mom = track_info.RecoIP.Momentum;
      mom.SetXYZ(reco_mom[0], reco_mom[1], reco_mom[2]);
    } else if (lmd_dim.dimension_options.track_param_type == LumiFit::LMD) {
      auto reco_pos = track_info.RecoLMD.Position;
      pos.SetXYZ(reco_pos[0], reco_pos[1], reco_pos[2]);
      auto reco_mom = track_info.RecoLMD.Momentum;
      mom.SetXYZ(reco_mom[0], reco_mom[1], reco_mom[2]);
    }
    theta = mom.Theta();
    phi = mom.Phi();
    theta_x = mom.X() / mom.Z();
    theta_y = mom.Y() / mom.Z();
  }

  else if (lmd_dim.dimension_options.track_type == LumiFit::DIFF_RECO_MC) {
    TVector3 mcpos(0.0, 0.0, 0.0);
    TVector3 mcmom(0.0, 0.0, 0.0);
    if (lmd_dim.dimension_options.track_param_type == LumiFit::IP) {
      auto mc_pos = track_info.MCIP.Position;
      mcpos.SetXYZ(mc_pos[0], mc_pos[1], mc_pos[2]);
      auto mc_mom = track_info.MCIP.Momentum;
      mcmom.SetXYZ(mc_mom[0], mc_mom[1], mc_mom[2]);
      auto reco_pos = track_info.RecoIP.Position;
      pos.SetXYZ(reco_pos[0], reco_pos[1], reco_pos[2]);
      auto reco_mom = track_info.RecoIP.Momentum;
      mom.SetXYZ(reco_mom[0], reco_mom[1], reco_mom[2]);
    } else if (lmd_dim.dimension_options.track_param_type == LumiFit::LMD) {
      auto mc_pos = track_info.MCLMD.Position;
      mcpos.SetXYZ(mc_pos[0], mc_pos[1], mc_pos[2]);
      auto mc_mom = track_info.MCLMD.Momentum;
      mcmom.SetXYZ(mc_mom[0], mc_mom[1], mc_mom[2]);
      auto reco_pos = track_info.RecoLMD.Position;
      pos.SetXYZ(reco_pos[0], reco_pos[1], reco_pos[2]);
      auto reco_mom = track_info.RecoLMD.Momentum;
      mom.SetXYZ(reco_mom[0], reco_mom[1], reco_mom[2]);
    }

    pos.SetXYZ(pos.X() - mcpos.X(), pos.Y() - mcpos.Y(), pos.Z() - mcpos.Z());
    theta = mom.Theta() - mcmom.Theta();
    phi = mom.Phi() - mcmom.Phi();
    if (phi > TMath::Pi())
      phi = phi - 2.0 * TMath::Pi();
    if (phi < TMath::Pi())
      phi = phi + 2.0 * TMath::Pi();

    theta_x = mom.X() / mom.Z() - mcmom.X() / mcmom.Z();
    theta_y = mom.Y() / mom.Z() - mcmom.Y() / mcmom.Z();
  }

  if (lmd_dim.dimension_options.dimension_type == LumiFit::X) {
    return pos.X();
  } else if (lmd_dim.dimension_options.dimension_type == LumiFit::Y) {
    return pos.Y();
  } else if (lmd_dim.dimension_options.dimension_type == LumiFit::Z) {
    return pos.Z();
  } else if (lmd_dim.dimension_options.dimension_type == LumiFit::T) {
    TLorentzVector track(mom, sqrt(pow(mom.Mag(), 2.0) +
                                   pow(pdg->GetParticle(-2212)->Mass(), 2.0)));
    return -(track - beam).M2();
  } else if (lmd_dim.dimension_options.dimension_type == LumiFit::THETA_X) {
    return theta_x;
  } else if (lmd_dim.dimension_options.dimension_type == LumiFit::THETA_Y) {
    return theta_y;
  } else if (lmd_dim.dimension_options.dimension_type == LumiFit::THETA) {
    return theta;
  } else if (lmd_dim.dimension_options.dimension_type == LumiFit::PHI) {
    return phi;
  } else
    return 0.0;
}
