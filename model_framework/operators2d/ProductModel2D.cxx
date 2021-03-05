/*
 * ProductModel2D.cxx
 *
 *  Created on: Jan 10, 2013
 *      Author: steve
 */

#include "ProductModel2D.h"

#include <iostream>

ProductModel2D::ProductModel2D(std::string name_,
                               std::shared_ptr<Model2D> first_,
                               std::shared_ptr<Model2D> second_)
    : Model2D(name_), first(first_), second(second_) {
  addModelToList(first);
  addModelToList(second);
}

void ProductModel2D::initModelParameters() {}

mydouble ProductModel2D::eval(const mydouble *x) const {
  mydouble result1(first->evaluate(x));
  if (result1 == 0.0)
    return 0.0;
  mydouble result2(second->evaluate(x));
  if (result2 == 0.0)
    return 0.0;
  return result1 * result2;
}

std::pair<mydouble, mydouble>
ProductModel2D::getUncertaincy(const mydouble *x) const {
  return std::make_pair(first->getUncertaincy(x).first * second->eval(x) +
                            second->getUncertaincy(x).first * first->eval(x),
                        first->getUncertaincy(x).second * second->eval(x) +
                            second->getUncertaincy(x).second * first->eval(x));
}

void ProductModel2D::updateDomain() {
  // first we need to check if user defined a domain for his models
  if (first->getVar1DomainRange() == 0 || second->getVar1DomainRange() == 0 ||
      first->getVar2DomainRange() == 0 || second->getVar2DomainRange() == 0) {
    std::cout
        << "Warning: Some of the models used for the multiplication have not"
           " defined any domains!"
        << std::endl;
  } else {
    setVar1Domain(
        std::max(first->getVar1DomainLowerBound(),
                 second->getVar1DomainLowerBound()),
        std::min(first->getVar1DomainLowerBound() + first->getVar1DomainRange(),
                 second->getVar1DomainLowerBound() +
                     second->getVar1DomainRange()));
    setVar2Domain(
        std::max(first->getVar2DomainLowerBound(),
                 second->getVar2DomainLowerBound()),
        std::min(first->getVar2DomainLowerBound() + first->getVar2DomainRange(),
                 second->getVar2DomainLowerBound() +
                     second->getVar2DomainRange()));
  }
}
