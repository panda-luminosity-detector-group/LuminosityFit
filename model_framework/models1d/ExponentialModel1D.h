/*
 * ExponentialModel1D.h
 *
 *  Created on: Apr 5, 2013
 *      Author: steve
 */

#ifndef EXPONENTIALMODEL1D_H_
#define EXPONENTIALMODEL1D_H_

#include "core/Model1D.h"

class ExponentialModel1D: public Model1D {
  private:
    std::shared_ptr<ModelPar> amplitude;
    std::shared_ptr<ModelPar> exp_factor;

  public:
    ExponentialModel1D(std::string name_);
    virtual ~ExponentialModel1D();

    mydouble eval(const mydouble *x) const;

    void initModelParameters();

    void updateDomain();
};

#endif /* EXPONENTIALMODEL1D_H_ */
