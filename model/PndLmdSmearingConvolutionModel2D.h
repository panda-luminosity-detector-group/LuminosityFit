#ifndef PNDLMDSMEARINGCONVOLUTIONMODEL2D_H_
#define PNDLMDSMEARINGCONVOLUTIONMODEL2D_H_

#include "PndLmdSmearingModel2D.h"
#include "core/Model2D.h"

class PndLmdSmearingConvolutionModel2D : public Model2D {
  std::shared_ptr<Model2D> unsmeared_model;
  std::shared_ptr<PndLmdSmearingModel2D> smearing_model;

  LumiFit::LmdDimension data_dim_x;
  LumiFit::LmdDimension data_dim_y;

  mydouble **model_grid;

  unsigned int nthreads;

  void generateModelGrid2D();
  void generateModelGrid2D(unsigned int index);

public:
  PndLmdSmearingConvolutionModel2D(
      std::string name_, std::shared_ptr<Model2D> unsmeared_model_,
      std::shared_ptr<PndLmdSmearingModel2D> smearing_model_,
      const LumiFit::LmdDimension &data_dim_x_,
      const LumiFit::LmdDimension &data_dim_y_);
  virtual ~PndLmdSmearingConvolutionModel2D();

  void initModelParameters();

  void injectModelParameter(std::shared_ptr<ModelPar> model_param);

  mydouble eval(const mydouble *x) const;

  void updateDomain();
};

#endif /* PNDLMDSMEARINGCONVOLUTIONMODEL2D_H_ */
