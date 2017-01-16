/*
 * PndLmdDivergenceSmearingModel2D.cxx
 *
 *  Created on: Nov 21, 2014
 *      Author: steve
 */

#include "LumiFitStructs.h"
#include "model/PndLmdDivergenceSmearingModel2D.h"
#include "ui/PndLmdRuntimeConfiguration.h"
#include "operators2d/integration/IntegralStrategyGSL2D.h"
#include "operators2d/integration/SimpleIntegralStrategy2D.h"

#include "boost/thread.hpp"

#include <limits>
#include <thread>
#include <future>
#include <functional>
#include <random>

PndLmdDivergenceSmearingModel2D::PndLmdDivergenceSmearingModel2D(
    shared_ptr<Model2D> divergence_model_,
    const LumiFit::LmdDimension& data_dim_x_,
    const LumiFit::LmdDimension& data_dim_y_) :
    divergence_model(divergence_model_), data_dim_x(data_dim_x_), data_dim_y(
        data_dim_y_), integral_precision(1e-5) {

}

PndLmdDivergenceSmearingModel2D::~PndLmdDivergenceSmearingModel2D() {
}

std::pair<double, double> PndLmdDivergenceSmearingModel2D::getBinsizes() const {
  return std::make_pair(data_dim_x.bin_size, data_dim_y.bin_size);
}

const std::vector<DifferentialCoordinateContribution>& PndLmdDivergenceSmearingModel2D::getListOfContributors(
    const double *x) const {
  return list_of_contributors;
}

void PndLmdDivergenceSmearingModel2D::generate2DDivergenceMap() {
  std::cout << "generating divergence map..." << std::endl;

  unsigned int nthreads =
      PndLmdRuntimeConfiguration::Instance().getNumberOfThreads();

  double div_bin_size_x = data_dim_x.bin_size;
  double div_bin_size_y = data_dim_y.bin_size;

  int div_bining_x = divergence_model->getVar1DomainRange() / div_bin_size_x
      / 2.0;
  int div_bining_y = divergence_model->getVar2DomainRange() / div_bin_size_y
      / 2.0;

  std::cout << "x domain range: " << divergence_model->getVar1DomainRange()
      << " || " << div_bin_size_x << std::endl;
  std::cout << "y domain range: " << divergence_model->getVar2DomainRange()
      << " || " << div_bin_size_y << std::endl;

  std::cout << "using " << (2 * div_bining_x + 1) << " x "
  << (2 * div_bining_y + 1) << " binning!" << std::endl;

  unsigned int list_counter(0);
  unsigned int pair_counter(0);

  unsigned int total_pairs((2 * div_bining_x + 1) * (2 * div_bining_y + 1));
  unsigned int pairs_per_thread(total_pairs / nthreads);
  if (pairs_per_thread * nthreads < total_pairs)
    ++pairs_per_thread;

  std::vector<std::vector<std::pair<int, int> > > xy_pairs_lists;
  xy_pairs_lists.resize(nthreads);

  for (int ibinx = -div_bining_x; ibinx <= div_bining_x; ++ibinx) {
    for (int ibiny = -div_bining_y; ibiny <= div_bining_y; ++ibiny) {
      xy_pairs_lists[list_counter].push_back(std::make_pair(ibinx, ibiny));
      ++pair_counter;
      if (pair_counter % pairs_per_thread == 0)
        list_counter++;
    }
  }

  std::cout << "pair and list counter: " << pair_counter << " " << list_counter
   << std::endl;
  std::cout << "with " << nthreads << " threads with " << pairs_per_thread
  << " pairs per thread" << std::endl;

  /* std::cout << "size: " << list_of_contributors.size() << std::endl;
   for (unsigned int i = 0; i < list_of_contributors.size(); ++i) {
   std::cout << list_of_contributors[i].contribution_factor << std::endl;
   }*/

  list_of_contributors.clear();
  list_of_contributors.reserve((2 * div_bining_x + 1) * (2 * div_bining_y + 1));

  std::vector<std::vector<DifferentialCoordinateContribution> > list_of_contributors_per_thread;
  list_of_contributors_per_thread.resize(nthreads);

  //std::vector<std::vector<DifferentialCoordinateContribution> > list_of_contributors_per_thread;
  //list_of_contributors_per_thread.resize(nthreads);

  optimizeNumericalIntegration(xy_pairs_lists);

  //boost::thread_group threads;
  std::vector<std::thread> thread_list;
  for (unsigned int i = 0; i < nthreads; i++) {
    //threads.create_thread(
    //    boost::bind(&PndLmdDivergenceSmearingModel2D::generate2DDivergenceMap,
    //        this, boost::cref(xy_pairs_lists[i]),
    //        boost::ref(list_of_contributors_per_thread[i])));
    /*std::packaged_task<void()> task(
     std::bind(&PndLmdDivergenceSmearingModel2D::generate2DDivergenceMap,
     this, std::cref(xy_pairs_lists[i]),
     std::ref(list_of_contributors_per_thread[i])));*/
    std::thread task(
        &PndLmdDivergenceSmearingModel2D::generate2DDivergenceMapPart, this,
        std::cref(xy_pairs_lists[i]),
        std::ref(list_of_contributors_per_thread[i]));
    thread_list.push_back(std::thread(std::move(task)));
  }
  //threads.join_all();

  // join all threads
  for (auto& thread : thread_list) {
    if (thread.joinable())
      thread.join();
  }

  for (unsigned int i = 0; i < list_of_contributors_per_thread.size(); ++i) {
    list_of_contributors.insert(list_of_contributors.end(),
        list_of_contributors_per_thread[i].begin(),
        list_of_contributors_per_thread[i].end());
  }

  std::cout << " number of divergence smearing contribution bins: "
      << list_of_contributors.size() << std::endl;
  /*for (unsigned int i = 0; i < list_of_contributors.size(); ++i) {
   std::cout << list_of_contributors[i].contribution_factor << std::endl;
   }*/
}

void PndLmdDivergenceSmearingModel2D::optimizeNumericalIntegration(
    const std::vector<std::vector<std::pair<int, int> > >& xy_pairs_lists) {
  unsigned int nthreads =
      PndLmdRuntimeConfiguration::Instance().getNumberOfThreads();

  /*shared_ptr<SimpleIntegralStrategy2D> integral_strategy(
      new SimpleIntegralStrategy2D());
  divergence_model->setIntegralStrategy(integral_strategy);

  unsigned int calls(7);

  integral_strategy->setUsedEvaluationGridConstant(calls);*/

  double div_bin_size_x = data_dim_x.bin_size;
  double div_bin_size_y = data_dim_y.bin_size;

  shared_ptr<IntegralStrategyGSL2D> integral_strategy(
      new IntegralStrategyGSL2D());
  divergence_model->setIntegralStrategy(integral_strategy);

  unsigned int calls(2);
  std::vector<std::vector<DataStructs::DimensionRange> > int_ranges(nthreads);
  std::vector<DataStructs::DimensionRange> int_range(2);
  std::vector<std::future<unsigned int> > future_list;
  std::vector<std::thread> thread_list;

  std::mt19937 gen;
  for (unsigned int i = 0; i < nthreads; i++) {
    if (xy_pairs_lists[i].size() > 0) {
      std::uniform_int_distribution<unsigned int> random_index(0,
          xy_pairs_lists[i].size() - 1);
      unsigned int index = random_index(gen);

      int_range[0].range_low = (xy_pairs_lists[i][index].first - 0.5)
          * div_bin_size_x;
      int_range[0].range_high = (xy_pairs_lists[i][index].first + 0.5)
          * div_bin_size_x;

      int_range[1].range_low = (xy_pairs_lists[i][index].second - 0.5)
          * div_bin_size_y;
      int_range[1].range_high = (xy_pairs_lists[i][index].second + 0.5)
          * div_bin_size_y;

      int_ranges[i] = int_range;

      std::packaged_task<unsigned int()> task(
          std::bind(&IntegralStrategyGSL2D::determineOptimalCallNumber,
              integral_strategy.get(), divergence_model.get(),
              std::cref(int_ranges[i]), integral_precision));
      future_list.push_back(task.get_future());
      thread_list.push_back(std::thread(std::move(task)));
    }
  }
// wait for futures and compute maximum number of calls

  for (auto& future : future_list) {
    unsigned int temp_calls = future.get();
    if (temp_calls > calls)
      calls = temp_calls;
  }

// join all threads
  for (auto& thread : thread_list) {
    if (thread.joinable())
      thread.join();
  }

  //std::cout << "using start call: " << calls << std::endl;
  integral_strategy->setStartNumberOfFunctionEvaluations(calls);
}

void PndLmdDivergenceSmearingModel2D::generate2DDivergenceMapPart(
    const std::vector<std::pair<int, int> >& xy_pairs,
    std::vector<DifferentialCoordinateContribution>& list_of_contributors_for_thread) const {

  std::vector<DataStructs::DimensionRange> int_range(2);

  std::set<DifferentialCoordinateContribution,
      PndLmdDivergenceSmearingModel2D::IntPairLess> temp_contribution_list;

  double div_bin_size_x = data_dim_x.bin_size;
  double div_bin_size_y = data_dim_y.bin_size;

  int div_bining_x = divergence_model->getVar1DomainRange() / div_bin_size_x
      / 2.0;
  int div_bining_y = divergence_model->getVar2DomainRange() / div_bin_size_y
      / 2.0;

  //std::cout << "num pairs: " << xy_pairs.size() << std::endl;
  for (unsigned int i = 0; i < xy_pairs.size(); ++i) {

    int_range[0].range_low = (xy_pairs[i].first - 0.5) * div_bin_size_x;
    int_range[0].range_high = (xy_pairs[i].first + 0.5) * div_bin_size_x;

    int_range[1].range_low = (xy_pairs[i].second - 0.5) * div_bin_size_y;
    int_range[1].range_high = (xy_pairs[i].second + 0.5) * div_bin_size_y;

    // calculate integral over the divergence bin
    mydouble smearing_probability = divergence_model->Integral(int_range,
        integral_precision);

    /*std::cout << "int range: " << int_range[0].range_low << " - "
     << int_range[0].range_high << " :: " << int_range[1].range_low << " - "
     << int_range[1].range_high << std::endl;
     std::cout << "smear prob: " << smearing_probability << std::endl;*/
    DifferentialCoordinateContribution dcc(xy_pairs[i].first,
        xy_pairs[i].second, smearing_probability);

    /*std::cout << xy_pairs[i].first << " and " << xy_pairs[i].second << " -> "
     << (2 * div_bining_y + 1) << " * " << (xy_pairs[i].first + div_bining_x)
     << " + " << (xy_pairs[i].second + div_bining_y) << std::endl;
     std::cout << "adding at "
     << (2 * div_bining_y + 1) * (xy_pairs[i].first + div_bining_x)
     + (xy_pairs[i].second + div_bining_y) << std::endl;*/

    /*list_of_contributors[(2 * div_bining_y + 1)
     * (xy_pairs[i].first + div_bining_x)
     + (xy_pairs[i].second + div_bining_y)] = dcc;*/
    list_of_contributors_for_thread.push_back(dcc);
  }

  //std::cout << "done!" << std::endl;
}

void PndLmdDivergenceSmearingModel2D::updateSmearingModel() {
  divergence_model->updateDomain();
  generate2DDivergenceMap();
}
