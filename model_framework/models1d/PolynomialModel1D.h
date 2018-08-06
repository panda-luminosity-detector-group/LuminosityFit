/*
 * PolynomialModel1D.h
 *
 *  Created on: Apr 5, 2013
 *      Author: steve
 */

#ifndef POLYNOMIALMODEL1D_H_
#define POLYNOMIALMODEL1D_H_

#include "core/Model1D.h"

class PolynomialModel1D: public Model1D {
  private:
    unsigned int order;
    std::vector<std::shared_ptr<ModelPar> > poly_factors;

  public:
    PolynomialModel1D(std::string name_, unsigned int order_);
    virtual ~PolynomialModel1D();

    mydouble eval(const mydouble *x) const;

    void initModelParameters();

    void updateDomain();
};

#endif /* POLYNOMIALMODEL1D_H_ */
