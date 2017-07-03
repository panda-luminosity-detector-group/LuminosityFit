#include "model/CachedModel2D.h"
#include "ui/PndLmdRuntimeConfiguration.h"
#include "operators2d/integration/IntegralStrategyGSL2D.h"
#include "operators2d/integration/SimpleIntegralStrategy2D.h"

#include "boost/thread.hpp"

#include <limits>
#include <thread>
#include <future>
#include <functional>
#include <random>

CachedModel2D::CachedModel2D(const std::string& name,
    shared_ptr<Model2D> model_, const LumiFit::LmdDimension& data_dim_x_,
    const LumiFit::LmdDimension& data_dim_y_) :
    Model2D(name), model(model_), data_dim_x(data_dim_x_), data_dim_y(
        data_dim_y_), integral_precision(1e-6) {
  nthreads = PndLmdRuntimeConfiguration::Instance().getNumberOfThreads();

  addModelToList(model);

  setVar1Domain(data_dim_x.dimension_range.getRangeLow(),
      data_dim_x.dimension_range.getRangeHigh());
  setVar2Domain(data_dim_y.dimension_range.getRangeLow(),
      data_dim_y.dimension_range.getRangeHigh());
  initializeModelGrid();
}

CachedModel2D::~CachedModel2D() {
  for (unsigned int i = 0; i < data_dim_x.bins; i++) {
    delete[] (model_grid[i]);
  }
  delete[] (model_grid);
}

void CachedModel2D::initializeModelGrid() {
  model_grid = new mydouble*[data_dim_x.bins];
  for (unsigned int i = 0; i < data_dim_x.bins; i++) {
    model_grid[i] = new mydouble[data_dim_y.bins];
  }

  mydouble div_bin_size_x = data_dim_x.bin_size;
  mydouble div_bin_size_y = data_dim_y.bin_size;
  inverse_bin_area = 1.0;
  inverse_bin_area = inverse_bin_area / div_bin_size_x / div_bin_size_y;

  int bins_x = data_dim_x.bins;
  int bins_y = data_dim_y.bins;

  unsigned int list_counter(0);
  unsigned int pair_counter(0);

  unsigned int total_pairs(bins_x * bins_y);
  unsigned int pairs_per_thread(total_pairs / nthreads);
  if (pairs_per_thread * nthreads < total_pairs)
    ++pairs_per_thread;

  int_ranges_lists.resize(nthreads);

  IntRange2D int_range;
  int_range.int_range.resize(2);
  for (unsigned int ibinx = 0; ibinx < bins_x; ++ibinx) {
    int_range.int_range[0].range_low = data_dim_x.dimension_range.getRangeLow()
        + div_bin_size_x * ibinx;
    int_range.int_range[0].range_high = data_dim_x.dimension_range.getRangeLow()
        + div_bin_size_x * (ibinx + 1);
    int_range.index_x = ibinx;
    for (unsigned int ibiny = 0; ibiny < bins_y; ++ibiny) {
      int_range.int_range[1].range_low =
          data_dim_x.dimension_range.getRangeLow() + div_bin_size_x * ibiny;
      int_range.int_range[1].range_high =
          data_dim_x.dimension_range.getRangeLow()
              + div_bin_size_x * (ibiny + 1);
      int_range.index_y = ibiny;

      int_ranges_lists[list_counter].push_back(int_range);

      ++pair_counter;
      if (pair_counter % pairs_per_thread == 0)
        list_counter++;
    }
  }

  optimizeNumericalIntegration();
}

void CachedModel2D::initModelParameters() {
}

void CachedModel2D::generateModelGrid2D() {
 // std::cout << "generating model grid...\n";
 // optimizeNumericalIntegration();

  // create threads and let them evaluate a part of the data
  boost::thread_group threads;

  for (unsigned int i = 0; i < nthreads; i++) {
    threads.create_thread(
        boost::bind(&CachedModel2D::generateModelGrid2D, this,
            boost::cref(int_ranges_lists[i])));
  }

  threads.join_all();
//  std::cout << "done!\n";
}

void CachedModel2D::optimizeNumericalIntegration() {
  double div_bin_size_x = data_dim_x.bin_size;
  double div_bin_size_y = data_dim_y.bin_size;

  //integral_strategy.reset(
  //    new IntegralStrategyGSL2D());
  shared_ptr<SimpleIntegralStrategy2D> integral_strategy(
      new SimpleIntegralStrategy2D());
  model->setIntegralStrategy(integral_strategy);

  unsigned int calls(5);
  /*std::vector<std::future<unsigned int> > future_list;
   std::vector<std::thread> thread_list;

   std::mt19937 gen;
   for (unsigned int i = 0; i < nthreads; i++) {
   if (int_ranges_lists[i].size() > 0) {
   std::uniform_int_distribution<unsigned int> random_index(0,
   int_ranges_lists[i].size() - 1);
   unsigned int index = random_index(gen);

   std::packaged_task<unsigned int()> task(
   std::bind(&IntegralStrategyGSL2D::determineOptimalCallNumber,
   integral_strategy.get(), model.get(),
   std::cref(int_ranges_lists[i][index].int_range), integral_precision));
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
   }*/

  //std::cout << "using start call: " << calls << std::endl;
  integral_strategy->setUsedEvaluationGridConstant(calls);
  //integral_strategy->setStartNumberOfFunctionEvaluations(calls);
}

void CachedModel2D::generateModelGrid2D(
    const std::vector<IntRange2D>& int_ranges) {
  mydouble x[2];

  shared_ptr<SimpleIntegralStrategy2D> test_integral_strategy(
      new SimpleIntegralStrategy2D());
  test_integral_strategy->setUsedEvaluationGridConstant(5);

  //std::cout << "num pairs: " << xy_pairs.size() << std::endl;
  for (unsigned int i = 0; i < int_ranges.size(); ++i) {
    //x[0] = int_ranges[i].int_range[0].getDimensionMean();
    //x[1] = int_ranges[i].int_range[1].getDimensionMean();
    //model_grid[int_ranges[i].index_x][int_ranges[i].index_y] = model->eval(x);

    // calculate integral over the model bin
    model_grid[int_ranges[i].index_x][int_ranges[i].index_y] = inverse_bin_area
        * model->Integral(int_ranges[i].int_range, integral_precision);

    //std::cout << int_ranges[i].index_x << ":" << int_ranges[i].index_y << " = "
    //    << model_grid[int_ranges[i].index_x][int_ranges[i].index_y]
    //    << std::endl;
  }
}

mydouble CachedModel2D::eval(const mydouble *x) const {
  int ix = (x[0] - data_dim_x.dimension_range.getRangeLow())
      / data_dim_x.bin_size;
  int iy = (x[1] - data_dim_y.dimension_range.getRangeLow())
      / data_dim_y.bin_size;

  if (ix >= data_dim_x.bins || iy >= data_dim_y.bins || ix < 0 || iy < 0) {
    /* std::cout << ix << "?=(" << x[0] << "-"
     << data_dim_x.dimension_range.getRangeLow() << ")/"
     << data_dim_x.bin_size << std::endl;*/
    return 0.0;
  }

  return model_grid[ix][iy];
}

void CachedModel2D::updateDomain() {
  // ok lets do a check if parameters have changed
  bool recalculate(false);
  for (auto model_par : model->getModelParameterSet().getModelParameterMap()) {
    //std::cout<<"checking if model parameter has changed "<<model_par.first.second<<std::endl;
    if (model_par.second->isModified()) {
      recalculate = true;
      break;
    }
  }

  if (recalculate) {
    generateModelGrid2D();
  }
}
