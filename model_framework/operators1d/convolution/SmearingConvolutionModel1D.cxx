/*
 * SmearingConvolutionModel1D.cxx
 *
 *  Created on: Jul 9, 2013
 *      Author: steve
 */

#include "SmearingConvolutionModel1D.h"

#include <iostream>

SmearingConvolutionModel1D::SmearingConvolutionModel1D(
    std::string name_, std::shared_ptr<Model1D> first_,
    std::shared_ptr<Model1D> second_)
    : Model1D(name_), first(first_), second(second_), divisions(300) {
  addModelToList(first);
  addModelToList(second);
}

SmearingConvolutionModel1D::~SmearingConvolutionModel1D() {
  // TODO Auto-generated destructor stub
}

void SmearingConvolutionModel1D::initModelParameters() {}

mydouble SmearingConvolutionModel1D::eval(const mydouble *x) const {
  // x[0] is the reconstructed value
  mydouble value = 0.0;
  // first divide the domain of the first model (that should be smeared) into
  // subintervals
  mydouble low = first->getDomain().first;
  mydouble high = first->getDomain().second;
  mydouble interval_width = first->getDomainRange() / divisions;

  std::vector<DataStructs::DimensionRange> temp_range_first;
  DataStructs::DimensionRange dr;
  temp_range_first.push_back(dr);
  std::vector<DataStructs::DimensionRange> temp_range_second;
  temp_range_second.push_back(dr);

  // std::cout<<"x0: "<<x[0]<<std::endl;
  // and loop over these invervals
  for (unsigned int i = 0; i < divisions; i++) {
    mydouble interval_low = (low + interval_width * i);
    // check if the second model is in range for the current subinterval
    // to reach the reconstructed value x[0]
    mydouble interval_center = interval_low + 0.5 * interval_width;

    second->evaluate(&interval_center); // this is to get the correct
                                        // parametrization models activated
    // second->getModelParameterSet().printInfo();

    mydouble smear_interval_center = x[0] - interval_center;
    mydouble smear_interval_low = smear_interval_center - 0.5 * interval_width;

    /*std::cout<<x[0]<<":
     "<<second->getDomain().first<<"-"<<second->getDomain().second<<std::endl;
     second->getModelParameterSet().printInfo();*/

    // if not in range skip this interval
    if (second->getDomainRange() == 0.0)
      continue;
    if (second->getDomain().second < smear_interval_low ||
        second->getDomain().first > smear_interval_low + interval_width)
      continue;

    // std::cout<<x[0]<< " "<<interval_low<<" "<<smear_interval_low<<std::endl;

    // integrate second (smearing) function in that smear interval range
    temp_range_second[0].range_low = smear_interval_low;
    temp_range_second[0].range_high = smear_interval_low + interval_width;
    mydouble int_second = second->Integral(temp_range_second, 1e-3);

    // integrate second (smearing) function in that smear interval range
    temp_range_first[0].range_low = interval_low;
    temp_range_first[0].range_high = low + interval_width * (i + 1);
    mydouble int_first = first->Integral(temp_range_first, 1e-3);

    value += int_first * int_second;
  }
  if (value != 0.0)
    value = value / interval_width;

  return value;
}

void SmearingConvolutionModel1D::updateDomain() {
  // first we need to check if user defined a domain for his models
  if (first->getDomainRange() == 0) {
    std::cout << "Warning: The domain of the model " << first->getName()
              << " used for the convolution is not defined!" << std::endl;
  } else if (second->getDomainRange() == 0) {
    std::cout << "Warning: The domain of the model " << second->getName()
              << " used for the convolution is not defined!" << std::endl;
  } else {
    setDomain(first->getDomain().first + second->getDomain().first,
              first->getDomain().second + second->getDomain().second);
  }
}
