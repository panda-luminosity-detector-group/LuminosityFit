#include "PndLmdSmearingConvolutionModel2D.h"
#include "ui/PndLmdRuntimeConfiguration.h"

#include <cmath>
#include <iomanip>

#include <boost/thread.hpp>

PndLmdSmearingConvolutionModel2D::PndLmdSmearingConvolutionModel2D(
    std::string name_, shared_ptr<Model2D> unsmeared_model_,
    shared_ptr<PndLmdSmearingModel2D> smearing_model_,
    const LumiFit::LmdDimension& data_dim_x_,
    const LumiFit::LmdDimension& data_dim_y_) :
    Model2D(name_), data_dim_x(data_dim_x_), data_dim_y(data_dim_y_) {
  nthreads = PndLmdRuntimeConfiguration::Instance().getNumberOfThreads();

  unsmeared_model = unsmeared_model_;
  smearing_model = smearing_model_;

  addModelToList(unsmeared_model);

  // setVar1Domain(-TMath::Pi(), TMath::Pi());
  // setVar2Domain(-TMath::Pi(), TMath::Pi());

  model_grid = new mydouble*[data_dim_x.bins];
  for (unsigned int i = 0; i < data_dim_x.bins; i++) {
    model_grid[i] = new mydouble[data_dim_y.bins];
  }

}

PndLmdSmearingConvolutionModel2D::~PndLmdSmearingConvolutionModel2D() {
  for (unsigned int i = 0; i < data_dim_x.bins; i++) {
    delete[] (model_grid[i]);
  }
  delete[] (model_grid);

}

void PndLmdSmearingConvolutionModel2D::initModelParameters() {
}

void PndLmdSmearingConvolutionModel2D::injectModelParameter(
    shared_ptr<ModelPar> model_param) {
  getModelParameterSet().addModelParameter(model_param);
}

void PndLmdSmearingConvolutionModel2D::generateModelGrid2D() {
  std::cout << "generating resolution smeared grid..." << std::endl;
  // create threads and let them evaluate a part of the data
  boost::thread_group threads;

  for (unsigned int i = 0; i < nthreads; i++) {
    threads.create_thread(
        boost::bind(&PndLmdSmearingConvolutionModel2D::generateModelGrid2D,
            this, i));
  }

  threads.join_all();
  std::cout << "done!" << std::endl;
}
void PndLmdSmearingConvolutionModel2D::generateModelGrid2D(unsigned int index) {
  auto const& res_param = smearing_model->getListOfContributors(index);

  mydouble xx[2];
  for (auto const& reco_bin : res_param) {
    std::vector<mydouble> numbers;
    numbers.reserve(reco_bin.contributor_coordinate_weight_list.size() + 1);
    numbers.push_back(0.0);
    for (auto const& mc_element_coordinate_and_weight : reco_bin.contributor_coordinate_weight_list) {
      xx[0] = mc_element_coordinate_and_weight.bin_center_x;
      xx[1] = mc_element_coordinate_and_weight.bin_center_y;
      mydouble integral_unsmeared_model = unsmeared_model->evaluate(xx);
      /*integral_unsmeared_model = integral_unsmeared_model
       * mc_element_coordinate_and_weight.area;*/
      //value = value
      //    + integral_unsmeared_model
      //        * mc_element_coordinate_and_weight.smear_weight;
      numbers.push_back(
          integral_unsmeared_model
              * mc_element_coordinate_and_weight.smear_weight);
     /* if (std::isnan(integral_unsmeared_model)
          || std::isnan(mc_element_coordinate_and_weight.smear_weight))
        std::cout << xx[0] << " : " << xx[1] << " -> "
            << integral_unsmeared_model << " * "
            << mc_element_coordinate_and_weight.smear_weight << std::endl;*/
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

    int ix = (reco_bin.reco_bin_x - data_dim_x.dimension_range.getRangeLow())
        / data_dim_x.bin_size;
    int iy = (reco_bin.reco_bin_y - data_dim_y.dimension_range.getRangeLow())
        / data_dim_y.bin_size;

//    std::cout<<res_param[i].reco_bin_x<<" - "<<data_dim_x.dimension_range.getRangeLow()<<std::endl;

 /*   if (std::isnan(sum_value))
      std::cout << ix << " " << iy << " " << sum_value << std::endl;*/
    model_grid[ix][iy] = sum_value;
  }
}

mydouble PndLmdSmearingConvolutionModel2D::eval(const mydouble *x) const {
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

  /*if(std::isnan(model_grid[ix][iy]))
    std::cout<<ix<<" "<<iy<<" is nan!\n";*/

  return model_grid[ix][iy];
}

void PndLmdSmearingConvolutionModel2D::updateDomain() {
  smearing_model->updateSmearingModel();
  generateModelGrid2D();
}
