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

struct ContributorCoordinateWeight {
  mydouble bin_center_x;
  mydouble bin_center_y;

  mydouble smear_weight;
};

struct RecoBinSmearingContributions {
  mydouble reco_bin_x;
  mydouble reco_bin_y;

  std::vector<ContributorCoordinateWeight> contributor_coordinate_weight_list;
};

class PndLmdSmearingModel2D {
  struct binrange {
    unsigned int x_bin_low;
    unsigned int x_bin_high;
    unsigned int y_bin_low;
    unsigned int y_bin_high;
  };

  std::vector<std::vector<RecoBinSmearingContributions>>
      smearing_parameterization_lists;

  const LumiFit::LmdDimension dim_x;
  const LumiFit::LmdDimension dim_y;

  std::vector<RecoBinSmearingContributions> createSmearingParameterizationPart(
      const std::vector<RecoBinSmearingContributions>
          &smearing_parameterization_,
      const binrange &br) const;

public:
  PndLmdSmearingModel2D(const LumiFit::LmdDimension &dimx_,
                        const LumiFit::LmdDimension &dimy_);
  virtual ~PndLmdSmearingModel2D();

  void
  setSmearingParameterization(const std::vector<RecoBinSmearingContributions>
                                  &smearing_parameterization_);

  virtual const std::vector<RecoBinSmearingContributions> &
  getListOfContributors(unsigned int index) const;

  virtual void updateSmearingModel();
};

#endif /* PNDLMDSMEARINGMODEL2D_H_ */
