#ifndef MODEL_CACHEDMODEL2D_H_
#define MODEL_CACHEDMODEL2D_H_

#include <core/Model2D.h>
#include "LumiFitStructs.h"
#include "operators2d/integration/IntegralStrategyGSL2D.h"

class CachedModel2D: public Model2D {
  struct IntRange2D {
    std::vector<DataStructs::DimensionRange> int_range;

    unsigned int index_x;
    unsigned int index_y;
  };

  unsigned int nthreads;
  double integral_precision;

  shared_ptr<Model2D> model;

  LumiFit::LmdDimension data_dim_x;
  LumiFit::LmdDimension data_dim_y;
  mydouble inverse_bin_area;

  mydouble **model_grid;

  std::vector<std::vector<IntRange2D> > int_ranges_lists;

  void initializeModelGrid();

  void generateModelGrid2D();
  void optimizeNumericalIntegration();
  void generateModelGrid2D(const std::vector<IntRange2D>& int_ranges);

public:
  CachedModel2D(const std::string& name, shared_ptr<Model2D> model_,
      const LumiFit::LmdDimension& data_dim_x_,
      const LumiFit::LmdDimension& data_dim_y_);
  virtual ~CachedModel2D();

  void initModelParameters();

  mydouble eval(const double *x) const;

  virtual void updateDomain();
};

#endif /* MODEL_CACHEDMODEL2D_H_ */
