#include "PndLmdSmearingConvolutionModel2D.h"

#include <cmath>
#include <iomanip>

PndLmdSmearingConvolutionModel2D::PndLmdSmearingConvolutionModel2D(
    std::string name_, shared_ptr<Model2D> unsmeared_model_,
    shared_ptr<PndLmdSmearingModel2D> smearing_model_) :
    Model2D(name_) {

  unsmeared_model = unsmeared_model_;
  smearing_model = smearing_model_;

  addModelToList(unsmeared_model);
}

PndLmdSmearingConvolutionModel2D::~PndLmdSmearingConvolutionModel2D() {
}

void PndLmdSmearingConvolutionModel2D::initModelParameters() {
}

void PndLmdSmearingConvolutionModel2D::injectModelParameter(
    shared_ptr<ModelPar> model_param) {
  getModelParameterSet().addModelParameter(model_param);
}

mydouble PndLmdSmearingConvolutionModel2D::eval(const double *x) const {
  //mydouble value(0.0);

  const std::vector<ContributorCoordinateWeight>& mc_element_contributors(
      smearing_model->getListOfContributors(x));

  std::vector<mydouble> numbers;
  numbers.reserve(mc_element_contributors.size());

  //std::cout<<"num contributors: "<<mc_element_contributors.size()<<std::endl;
  double xx[2];
  for (unsigned int contributor_index = 0;
      contributor_index < mc_element_contributors.size(); ++contributor_index) {
    const ContributorCoordinateWeight &mc_element_coordinate_and_weight(
        mc_element_contributors[contributor_index]);
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
    //std::cout<<integral_unsmeared_model<< " * " << mc_element_coordinate_and_weight.smear_weight<<std::endl;
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

  mydouble sum_value(0.0);
  if(numbers.size() == 1)
    sum_value=numbers[0];
  if(numbers.size() == 2)
    sum_value = numbers[0] + numbers[1];

  /*if(value != 0) {
    if(std::fabs((value-sum_value)/sum_value) > 0.001)
      std::cout << std::setprecision(9) << "compare standard summation: " << value
      << " with pairwise summation: " << sum_value << std::endl;
  }*/

  //if(-0.0042 > x[0] && x[0] > -0.0044 && -0.0081 > x[1] && x[1] > -0.0083)
  //  std::cout<<x[0]<<":"<<x[1]<<"  value: "<<value<<std::endl;

  return sum_value;
}

void PndLmdSmearingConvolutionModel2D::updateDomain() {
  smearing_model->updateSmearingModel();
}
