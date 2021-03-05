/*
 * ProductModel2D.h
 *
 *  Created on: Jan 10, 2013
 *      Author: steve
 */

#ifndef PRODUCTMODEL2D_H_
#define PRODUCTMODEL2D_H_

#include "core/Model2D.h"

class ProductModel2D : public Model2D {
private:
  std::shared_ptr<Model2D> first, second;

public:
  ProductModel2D(std::string name_, std::shared_ptr<Model2D> first_,
                 std::shared_ptr<Model2D> second_);

  void initModelParameters();

  mydouble eval(const mydouble *x) const;

  void updateDomain();

  virtual std::pair<mydouble, mydouble> getUncertaincy(const mydouble *x) const;
};

#endif /* PRODUCTMODEL2D_H_ */
