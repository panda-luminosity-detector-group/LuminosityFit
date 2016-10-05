/*
 * PndLmdSmearingModel2D.h
 *
 *  Created on: Nov 21, 2014
 *      Author: steve
 */

#ifndef PNDLMDSMEARINGMODEL2D_H_
#define PNDLMDSMEARINGMODEL2D_H_

#include "LumiFitStructs.h"

#include <map>

#include "boost/thread/mutex.hpp"

struct ContributorCoordinateWeight {
  double bin_center_x;
  double bin_center_y;

  double area;
  double smear_weight;
};

struct RecoBinSmearingContributions {
  double reco_bin_x;
  double reco_bin_y;

  std::vector<ContributorCoordinateWeight> contributor_coordinate_weight_list;
};

class PndLmdSmearingModel2D {
  std::vector<ContributorCoordinateWeight> empty_contribution_list;
  double search_distance_x;
  double search_distance_y;

  unsigned int max_number_of_hints;
  std::vector<unsigned int> last_found_neighbour_indices;
  boost::mutex neighbour_index_list_lock;

  std::vector<RecoBinSmearingContributions> smearing_parameterization;

  const std::vector<ContributorCoordinateWeight>& findNearestNeighbour(
      const double *x);

  void determineSearchDistance();

public:
  PndLmdSmearingModel2D();
  virtual ~PndLmdSmearingModel2D();

  void setSmearingParameterization(
      const std::vector<RecoBinSmearingContributions>& smearing_parameterization_);
  void setSearchDistances(double search_distance_x_, double search_distance_y_);

  virtual const std::vector<ContributorCoordinateWeight>& getListOfContributors(
      const double *x);

  virtual void updateSmearingModel();
};

#endif /* PNDLMDSMEARINGMODEL2D_H_ */
