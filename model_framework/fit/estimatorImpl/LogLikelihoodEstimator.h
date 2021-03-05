#ifndef LOGLIKELIHOODESTIMATOR_H_
#define LOGLIKELIHOODESTIMATOR_H_

#include "fit/ModelEstimator.h"

class LogLikelihoodEstimator : public ModelEstimator {
private:
public:
  LogLikelihoodEstimator();
  virtual ~LogLikelihoodEstimator();

  // the likelihood function
  mydouble eval(std::shared_ptr<Data> data);
};

#endif /* LOGLIKELIHOODESTIMATOR_H_ */
