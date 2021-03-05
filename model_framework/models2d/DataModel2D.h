#ifndef DATAMODEL2D_H_
#define DATAMODEL2D_H_

#include "core/Model2D.h"

class DataModel2D : public Model2D {
private:
  mydouble grid_spacing[2];
  unsigned int cell_count[2];
  mydouble *data;

  mydouble domain_low[2];
  mydouble domain_high[2];

  mydouble grid_density;

  ModelStructs::InterpolationType intpol_type;

  std::shared_ptr<ModelPar> offset_x;
  std::shared_ptr<ModelPar> offset_y;

  // function pointer used to switch between different algorithms for
  // interpolation
  typedef mydouble (DataModel2D::*function)(const mydouble *x) const;

  function model_func;

  std::pair<mydouble, bool> getCellSpacing(const std::set<mydouble> &values);

public:
  DataModel2D(std::string name_,
              ModelStructs::InterpolationType type = ModelStructs::LINEAR);
  DataModel2D(const DataModel2D &data_model_);
  virtual ~DataModel2D();

  void setData(const std::map<std::pair<mydouble, mydouble>, mydouble> &data_);

  void setIntpolType(ModelStructs::InterpolationType intpol_type_);

  mydouble evaluateConstant(const mydouble *x) const;
  mydouble evaluateLinear(const mydouble *x) const;

  mydouble eval(const mydouble *x) const;

  void initModelParameters();

  void updateDomain();

  DataModel2D &operator=(const DataModel2D &data_model_);
};

#endif /* DATAMODEL2D_H_ */
