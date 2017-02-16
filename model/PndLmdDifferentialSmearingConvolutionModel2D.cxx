#include "PndLmdDifferentialSmearingConvolutionModel2D.h"
#include "ui/PndLmdRuntimeConfiguration.h"

#include <cmath>
#include <iostream>
#include <iomanip>

#include <boost/thread.hpp>

#include "TMath.h"

PndLmdDifferentialSmearingConvolutionModel2D::PndLmdDifferentialSmearingConvolutionModel2D(
    std::string name_, shared_ptr<Model2D> unsmeared_model_,
    shared_ptr<PndLmdDivergenceSmearingModel2D> smearing_model_,
    const LumiFit::LmdDimension& data_dim_x_,
    const LumiFit::LmdDimension& data_dim_y_, unsigned int combine_factor_) :
    Model2D(name_), data_dim_x(data_dim_x_), data_dim_y(data_dim_y_), combine_factor(
        combine_factor_) {
  nthreads = PndLmdRuntimeConfiguration::Instance().getNumberOfThreads();

  unsmeared_model = unsmeared_model_;
  smearing_model = smearing_model_;

  binsizes = smearing_model->getBinsizes();
  area_xy = binsizes.first * binsizes.second;

  calc_data_dim_x = data_dim_x;
  calc_data_dim_x.bins *= combine_factor;
  calc_data_dim_x.calculateBinSize();
  calc_data_dim_y = data_dim_y;
  calc_data_dim_y.bins *= combine_factor;
  calc_data_dim_y.calculateBinSize();

  addModelToList(unsmeared_model);

  setVar1Domain(-TMath::Pi(), TMath::Pi());
  setVar2Domain(-TMath::Pi(), TMath::Pi());

  unsigned int bins_per_thread_x(calc_data_dim_x.bins / nthreads);
  unsigned int bins_per_thread_y(calc_data_dim_y.bins / nthreads);

  bool use_x_splitting(true);
  if (calc_data_dim_x.bins % nthreads != 0) {
    if (calc_data_dim_y.bins % nthreads == 0)
      use_x_splitting = false;
    else {
      if (calc_data_dim_x.bins < 0 && calc_data_dim_y.bins < 0) {
        if (calc_data_dim_y.bins > calc_data_dim_x.bins) {
          use_x_splitting = false;
          nthreads = calc_data_dim_y.bins;
        } else {
          nthreads = calc_data_dim_x.bins;
        }
      } else {
        if (calc_data_dim_y.bins % nthreads > calc_data_dim_y.bins % nthreads) {
          use_x_splitting = false;
          bins_per_thread_y++;
        }

        else {
          bins_per_thread_x++;
        }
      }
    }
  }
  binrange br;
  br.x_bin_low = 0;
  br.x_bin_high = calc_data_dim_x.bins;
  br.y_bin_low = 0;
  br.y_bin_high = calc_data_dim_y.bins;
  for (unsigned int i = 0; i < nthreads; ++i) {
    if (use_x_splitting) {
      br.x_bin_low = i * bins_per_thread_x;
      br.x_bin_high = (i + 1) * bins_per_thread_x;
      if (i == nthreads - 1)
        br.x_bin_high = calc_data_dim_x.bins;
    } else {
      br.y_bin_low = i * bins_per_thread_y;
      br.y_bin_high = (i + 1) * bins_per_thread_y;
      if (i == nthreads - 1)
        br.y_bin_high = calc_data_dim_y.bins;
    }
    list_of_bin_ranges.push_back(br);
  }

  model_grid = new mydouble*[data_dim_x.bins];
  for (unsigned int i = 0; i < data_dim_x.bins; i++) {
    model_grid[i] = new mydouble[data_dim_y.bins];
  }
  fine_model_grid = new mydouble*[calc_data_dim_x.bins];
  for (unsigned int i = 0; i < calc_data_dim_x.bins; i++) {
    fine_model_grid[i] = new mydouble[calc_data_dim_y.bins];
  }

  previous_model_grid = new mydouble*[data_dim_x.bins];
  for (unsigned int i = 0; i < data_dim_x.bins; i++) {
    previous_model_grid[i] = new mydouble[data_dim_y.bins];
  }
}

PndLmdDifferentialSmearingConvolutionModel2D::~PndLmdDifferentialSmearingConvolutionModel2D() {
  for (unsigned int i = 0; i < data_dim_x.bins; i++) {
    delete[] (model_grid[i]);
  }
  delete[] (model_grid);

  for (unsigned int i = 0; i < calc_data_dim_x.bins; i++) {
    delete[] (fine_model_grid[i]);
  }
  delete[] (fine_model_grid);

  for (unsigned int i = 0; i < data_dim_x.bins; i++) {
    delete[] (previous_model_grid[i]);
  }
  delete[] (previous_model_grid);
}

void PndLmdDifferentialSmearingConvolutionModel2D::initModelParameters() {
}

void PndLmdDifferentialSmearingConvolutionModel2D::injectModelParameter(
    shared_ptr<ModelPar> model_param) {
  getModelParameterSet().addModelParameter(model_param);
}

void PndLmdDifferentialSmearingConvolutionModel2D::generateModelGrid2D() {
  std::cout << "generating divergence smeared grid..." << std::endl;
  // create threads and let them evaluate a part of the data
  boost::thread_group threads;

  for (unsigned int i = 0; i < nthreads; i++) {
    threads.create_thread(
        boost::bind(
            &PndLmdDifferentialSmearingConvolutionModel2D::generateModelGrid2D,
            this, boost::cref(list_of_bin_ranges[i])));
  }

  threads.join_all();
  std::cout << "done!" << std::endl;

  if (combine_factor > 1) {
    // now merge fine model grid to coarser grid
    for (unsigned int ix = 0; ix < calc_data_dim_x.bins;
        ix = ix + combine_factor) {
      for (unsigned int iy = 0; iy < calc_data_dim_y.bins;
          iy = iy + combine_factor) {
        mydouble temp(fine_model_grid[ix][iy]);
        temp += fine_model_grid[ix][iy + 1];
        mydouble temp2(fine_model_grid[ix + 1][iy]);
        temp2 += fine_model_grid[ix + 1][iy + 1];
        model_grid[ix / combine_factor][iy / combine_factor] = (temp + temp2)
            / (combine_factor * combine_factor);
      }
    }
    evaluation_grid = model_grid;
  } else {
    evaluation_grid = fine_model_grid;
  }

  /*mydouble mean_relative_error(0.0);
   unsigned int counter(0);
   std::cout << "comparing with previous grid...\n";
   for (unsigned int i = 0; i < data_dim_x.bins; ++i) {
   for (unsigned int j = 0; j < data_dim_y.bins; ++j) {
   if (model_grid[i][j] != 0.0) {
   mean_relative_error += std::abs(
   (model_grid[i][j] - previous_model_grid[i][j]) / model_grid[i][j]);
   ++counter;
   }
   previous_model_grid[i][j] = model_grid[i][j];
   }
   }
   std::cout << "mean relative error to previous variation:"
   << mean_relative_error / counter << std::endl;*/

  /*const auto &asdf = getModelParameterSet().getFreeModelParameters();
   for (const auto blub : asdf) {
   std::cout << blub.first.first << " : " << blub.first.second << ":  "
   << blub.second->getValue() << std::endl;
   }
   const auto &asdf2 =
   smearing_model->divergence_model->getModelParameterSet().getFreeModelParameters();
   for (const auto blub : asdf2) {
   std::cout << blub.first.first << " : " << blub.first.second << ":  "
   << blub.second->getValue() << std::endl;
   }*/
}

void PndLmdDifferentialSmearingConvolutionModel2D::generateModelGrid2D(
    const binrange &br) {
  mydouble x[2];
  //std::cout<<br.x_bin_low<<" "<<br.x_bin_high<<std::endl;
  //std::cout<<br.y_bin_low<<" "<<br.y_bin_high<<std::endl;

  for (unsigned int ix = br.x_bin_low; ix < br.x_bin_high; ix++) {
    x[0] = calc_data_dim_x.dimension_range.getRangeLow()
        + calc_data_dim_x.bin_size * (0.5 + ix);
    for (unsigned int iy = br.y_bin_low; iy < br.y_bin_high; iy++) {
      x[1] = calc_data_dim_y.dimension_range.getRangeLow()
          + calc_data_dim_y.bin_size * (0.5 + iy);

      //mydouble value(0.0);

      const std::vector<DifferentialCoordinateContribution> &mc_element_contributors =
          smearing_model->getListOfContributors(x);

      std::vector<mydouble> numbers;
      numbers.reserve(mc_element_contributors.size());


      //std::cout<<"contributors: "<<mc_element_contributors.size()<<std::endl;
      std::vector<DifferentialCoordinateContribution>::const_iterator mc_element_it;
      for (mc_element_it = mc_element_contributors.begin();
          mc_element_it != mc_element_contributors.end(); ++mc_element_it) {
        mydouble xx[2];
        xx[0] = x[0] - binsizes.first * mc_element_it->coordinate_delta.first;
        xx[1] = x[1] - binsizes.second * mc_element_it->coordinate_delta.second;
        //The negative sign in the above equations is crucial!!!
        //So we calculate the final value of one single bin x[] from all its neighbouring bins!
        //We turn around the definition so that we can use multi-threading!
        mydouble integral_unsmeared_model = unsmeared_model->evaluate(xx);

        //integral_unsmeared_model = integral_unsmeared_model * area_xy;
        numbers.push_back(
            integral_unsmeared_model * mc_element_it->contribution_factor);
        //value = value + numbers.back();

        /*if (value != value) {
         std::cout << xx[0] << ", " << xx[1] << std::endl;
         std::cout << integral_unsmeared_model << " " << mc_element_it->contribution_factor
         << std::endl;
         }*/

        /*if (fabs(x[0] + 0.00615) < 0.0001 && fabs(x[1] + 0.00705) < 0.0001) {
         std::cout << xx[0] << ", " << xx[1] << std::endl;
         std::cout << integral_unsmeared_model << " " << mc_element_it->second
         << std::endl;
         }*/
      }
      while (numbers.size() > 2) {
        std::vector<mydouble> temp_sum;
        temp_sum.reserve(numbers.size() / 2 + 1);
        if (numbers.size() % 2 == 0) {
          for (unsigned int i = 0; i < numbers.size(); i = i + 2) {
            temp_sum.push_back(numbers[i] + numbers[i + 1]);
          }
        } else {
          for (unsigned int i = 0; i < numbers.size() - 1; i = i + 2) {
            temp_sum.push_back(numbers[i] + numbers[i + 1]);
          }
          temp_sum.push_back(numbers.back());
        }
        numbers = temp_sum;
      }
      mydouble sum_value(numbers[0]);
      if (numbers.size() == 2)
        sum_value += numbers[1];

      /*if(value != 0) {
       if(std::fabs((value-sum_value)/sum_value) > 0.001)
       std::cout<<std::setprecision(9) << "compare standard summation: "<<value<<" with pairwise summation: "<<sum_value<<std::endl;
       }*/
      /*if (fabs(x[0] + 0.00615) < 0.0001 && fabs(x[1] + 0.00705) < 0.0001) {
       std::cout << x[0] << ", " << x[1] << " value: " << value << std::endl;
       std::cout << "contribution size: " << mc_element_contributors.size()
       << std::endl;
       }*/

      fine_model_grid[ix][iy] = sum_value;
    }
  }
}

mydouble PndLmdDifferentialSmearingConvolutionModel2D::eval(
    const mydouble *x) const {
  int ix = (x[0] - data_dim_x.dimension_range.getRangeLow())
      / data_dim_x.bin_size;
  int iy = (x[1] - data_dim_y.dimension_range.getRangeLow())
      / data_dim_y.bin_size;

  /*std::cout << ix << " " << iy << " " << data_dim_x.bins << " "
   << data_dim_y.bins << " " << x[0] << " "
   << data_dim_x.dimension_range.getRangeLow() << " - "
   << data_dim_x.dimension_range.getRangeHigh() << " " << x[1] << " "
   << data_dim_y.dimension_range.getRangeLow() << " - "
   << data_dim_y.dimension_range.getRangeHigh() << std::endl;*/

  if (ix >= data_dim_x.bins || iy >= data_dim_y.bins || ix < 0 || iy < 0)
    return 0.0;

  return evaluation_grid[ix][iy];
}

void PndLmdDifferentialSmearingConvolutionModel2D::updateDomain() {
  smearing_model->updateSmearingModel();
  unsmeared_model->updateDomain();
  generateModelGrid2D();
}
