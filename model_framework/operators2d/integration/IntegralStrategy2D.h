#ifndef INTEGRALSTRATEGY2D_H_
#define INTEGRALSTRATEGY2D_H_

#include <vector>

#include "core/Model2D.h"
#include "fit/data/DataStructs.h"

class IntegralStrategy2D {
public:
  IntegralStrategy2D();
  virtual ~IntegralStrategy2D();

  virtual mydouble
  Integral(Model2D *model2d,
           const std::vector<DataStructs::DimensionRange> &ranges,
           mydouble precision) = 0;
};

#endif /* INTEGRALSTRATEGY2D_H_ */
