/*
 * PndLmdSmearingModel2D.cxx
 *
 *  Created on: Nov 21, 2014
 *      Author: steve
 */

#include "ui/PndLmdRuntimeConfiguration.h"
#include <model/PndLmdSmearingModel2D.h>

PndLmdSmearingModel2D::PndLmdSmearingModel2D(const LumiFit::LmdDimension &dimx_,
                                             const LumiFit::LmdDimension &dimy_)
    : dim_x(dimx_), dim_y(dimy_) {}

PndLmdSmearingModel2D::~PndLmdSmearingModel2D() {}

void PndLmdSmearingModel2D::setSmearingParameterization(
    const std::vector<RecoBinSmearingContributions>
        &smearing_parameterization_) {
  unsigned int nthreads =
      PndLmdRuntimeConfiguration::Instance().getNumberOfThreads();

  std::cout << "chopping smearing parameterization for multithreading...\n";
  unsigned int bins_per_thread_x(dim_x.bins / nthreads);
  unsigned int bins_per_thread_y(dim_y.bins / nthreads);

  bool use_x_splitting(true);
  if (dim_x.bins % nthreads != 0) {
    if (dim_y.bins % nthreads == 0)
      use_x_splitting = false;
    else {
      if (dim_y.bins % nthreads > dim_x.bins % nthreads) {
        use_x_splitting = false;
        bins_per_thread_y++;
      } else {
        bins_per_thread_x++;
      }
    }
  }

  binrange br;
  br.x_bin_low = 0;
  br.x_bin_high = dim_x.bins;
  br.y_bin_low = 0;
  br.y_bin_high = dim_y.bins;
  for (unsigned int i = 0; i < nthreads; ++i) {
    if (use_x_splitting) {
      br.x_bin_low = i * bins_per_thread_x;
      br.x_bin_high = (i + 1) * bins_per_thread_x;
      if (i == nthreads - 1)
        br.x_bin_high = dim_x.bins;
    } else {
      br.y_bin_low = i * bins_per_thread_y;
      br.y_bin_high = (i + 1) * bins_per_thread_y;
      if (i == nthreads - 1)
        br.y_bin_high = dim_y.bins;
    }
    smearing_parameterization_lists.push_back(
        createSmearingParameterizationPart(smearing_parameterization_, br));
  }
  std::cout << "done!\n";
}

std::vector<RecoBinSmearingContributions>
PndLmdSmearingModel2D::createSmearingParameterizationPart(
    const std::vector<RecoBinSmearingContributions> &smearing_parameterization_,
    const binrange &br) const {
  std::vector<RecoBinSmearingContributions> smearing_param_part;

  mydouble x[2];
  for (unsigned int ix = br.x_bin_low; ix < br.x_bin_high; ix++) {
    x[0] = dim_x.dimension_range.getRangeLow() + dim_x.bin_size * (0.5 + ix);
    for (unsigned int iy = br.y_bin_low; iy < br.y_bin_high; iy++) {
      x[1] = dim_y.dimension_range.getRangeLow() + dim_y.bin_size * (0.5 + iy);
      auto result = std::find_if(
          smearing_parameterization_.begin(), smearing_parameterization_.end(),
          [&](const RecoBinSmearingContributions &rec_bin) {
            if (std::fabs(rec_bin.reco_bin_x - x[0]) < dim_x.bin_size / 2 &&
                std::fabs(rec_bin.reco_bin_y - x[1]) < dim_y.bin_size / 2)
              return true;
            else
              return false;
          });
      if (result != smearing_parameterization_.end()) {
        smearing_param_part.push_back(*result);
        // std::cout << "found corresponding reco bin!\n";
      } else {
        // std::cout << "did not find a corresponding reco bin...\n";
        RecoBinSmearingContributions temp;
        temp.reco_bin_x = x[0];
        temp.reco_bin_y = x[1];
        smearing_param_part.push_back(temp);
      }
    }
  }
  return smearing_param_part;
}

const std::vector<RecoBinSmearingContributions> &
PndLmdSmearingModel2D::getListOfContributors(unsigned int index) const {
  return smearing_parameterization_lists[index];
}

void PndLmdSmearingModel2D::updateSmearingModel() {}
