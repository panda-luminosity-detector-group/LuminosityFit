#ifndef UNBINNEDLOGLIKELIHOODESTIMATOR_H_
#define UNBINNEDLOGLIKELIHOODESTIMATOR_H_

#include "fit/ModelEstimator.h"

class UnbinnedLogLikelihoodEstimator : public ModelEstimator {
private:
  mydouble last_integral;
  bool skip_integral;

  int integral_skip_counter;
  int integral_skip_amount;

public:
  UnbinnedLogLikelihoodEstimator();
  virtual ~UnbinnedLogLikelihoodEstimator();

  // the likelihood function
  mydouble eval(std::shared_ptr<Data> data);
};

#endif /* UNBINNEDLOGLIKELIHOODESTIMATOR_H_ */
