/*
 * AdditionModel1D.cxx
 *
 *  Created on: Apr 10, 2013
 *      Author: steve
 */

#include "AdditionModel1D.h"

#include <iostream>

AdditionModel1D::AdditionModel1D(std::string name_,
                                 std::shared_ptr<Model1D> first_,
                                 std::shared_ptr<Model1D> second_)
    : Model1D(name_), first(first_), second(second_) {

  addModelToList(first);
  addModelToList(second);
}

AdditionModel1D::~AdditionModel1D() {
  
}

void AdditionModel1D::initModelParameters() {}

mydouble AdditionModel1D::eval(const mydouble *x) const {
  return add(first, second, x);
}

void AdditionModel1D::updateDomain() {
  // first we need to check if user defined a domain for his models
  if (first->getDomainRange() == 0) {
    std::cout << "Warning: The domain of the model " << first->getName()
              << " used for the addition is not defined!" << std::endl;
  } else if (second->getDomainRange() == 0) {
    std::cout << "Warning: The domain of the model " << second->getName()
              << " used for the addition is not defined!" << std::endl;
  } else {
    setDomain(std::min(first->getDomain().first, second->getDomain().first),
              std::max(first->getDomain().second, second->getDomain().second));
  }
}
