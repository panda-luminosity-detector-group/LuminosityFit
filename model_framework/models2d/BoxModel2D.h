#ifndef BOXMODEL2D_H_
#define BOXMODEL2D_H_

#include "core/Model2D.h"

class BoxModel2D : public Model2D {
private:
  std::shared_ptr<ModelPar> amplitude;
  std::shared_ptr<ModelPar> lower_edge_var1;
  std::shared_ptr<ModelPar> upper_edge_var1;
  std::shared_ptr<ModelPar> lower_edge_var2;
  std::shared_ptr<ModelPar> upper_edge_var2;

public:
  BoxModel2D(std::string name_);
  virtual ~BoxModel2D();

  mydouble eval(const mydouble *x) const;

  void initModelParameters();

  void updateDomain();
};

#endif /* STEPFUNCTION1D_H_ */
