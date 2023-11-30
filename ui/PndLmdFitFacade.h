/*
 * PndLmdFitFacade.h
 *
 *  Created on: Aug 30, 2013
 *      Author: steve
 */

#ifndef PNDLMDFITFACADE_H_
#define PNDLMDFITFACADE_H_

#include "LumiFitStructs.h"
#include "PndLmdRuntimeConfiguration.h"
#include "data/PndLmdFitDataBundle.h"
#include "fit/ModelFitFacade.h"
#include "fit/data/ROOT/ROOTDataHelper.h"
#include "model/PndLmdModelFactory.h"

#include <vector>

class PndLmdVertexData;
class PndLmdHistogramData;

#include "boost/property_tree/ptree_fwd.hpp"

class PndLmdFitFacade {
private:
  std::vector<PndLmdAcceptance> acceptance_pool;
  std::set<PndLmdHistogramData> resolution_pool;
  std::set<PndLmdMapData> resolution_map_pool;

  const PndLmdRuntimeConfiguration &lmd_runtime_config;

  // ROOT data helper class
  ROOTDataHelper data_helper;

  ModelFitFacade model_fit_facade;

  PndLmdModelFactory model_factory;

  static void signalHandler(int signum);

  std::shared_ptr<Data>
  createData1D(const PndLmdHistogramData &lmd_hist_data) const;
  std::shared_ptr<Data>
  createData2D(const PndLmdHistogramData &lmd_hist_data) const;

  void
  saveFittedObjectsToFile(std::vector<PndLmdAngularData> &lmd_data_vec) const;

  EstimatorOptions constructEstimatorOptionsFromConfig(
      const boost::property_tree::ptree &pt) const;
  std::set<std::string, ModelStructs::string_comp>
  constructFreeFitParameterListFromConfig(
      const boost::property_tree::ptree &pt) const;

  void freeParametersForModel(std::shared_ptr<Model> current_model,
                              const PndLmdFitOptions &fit_opts) const;

  void addBeamParametersToFreeParameterList(
      PndLmdFitOptions &fit_opts,
      const boost::property_tree::ptree &model_opt_ptree) const;

  PndLmdFitOptions createFitOptions(const PndLmdAbstractData &lmd_data) const;

public:
  PndLmdFitFacade();
  virtual ~PndLmdFitFacade();

  void setModelFactoryAcceptence(const PndLmdAcceptance &lmd_acc);
  void
  setModelFactoryResolutions(const std::vector<PndLmdHistogramData> &lmd_res);
  void setModelFactoryResolutionMap(const PndLmdMapData &lmd_res);

  void addAcceptencesToPool(const std::vector<PndLmdAcceptance> &lmd_acc);
  void addResolutionsToPool(const std::vector<PndLmdHistogramData> &lmd_res);
  void addResolutionMapsToPool(const std::vector<PndLmdMapData> &lmd_res);

  void clearPools();

  double calcHistIntegral(const TH1D *hist,
                          std::vector<DataStructs::DimensionRange> range) const;
  double calcHistIntegral(const TH2D *hist,
                          std::vector<DataStructs::DimensionRange> range) const;

  std::vector<DataStructs::DimensionRange>
  calcRange(const PndLmdAbstractData &lmd_abs_data,
            const EstimatorOptions &est_options) const;

  void initBeamParametersForModel(
      std::shared_ptr<Model> current_model,
      const boost::property_tree::ptree &model_opt_ptree) const;
 
  void initIPForModel(
      std::shared_ptr<Model> current_model,
      const boost::property_tree::ptree &model_opt_ptree) const;

  PndLmdFitDataBundle
  doLuminosityFits(std::vector<PndLmdAngularData> &lmd_data_vec);

  void fitElasticPPbar(PndLmdAngularData &lmd_data);

  std::shared_ptr<Model> generateModel(const PndLmdAngularData &lmd_data);
  std::shared_ptr<Model> generateModel(const PndLmdAngularData &lmd_data,
                                       const PndLmdFitOptions &fit_options);

  void scanEstimatorSpace(PndLmdHistogramData &lmd_hist_data,
                          const PndLmdFitOptions &fit_options,
                          const std::vector<std::string> &variable_names);
  void doFit(PndLmdHistogramData &lmd_hist_data,
             const PndLmdFitOptions &fit_options);

  void fitVertexData(std::vector<PndLmdHistogramData> &lmd_data);
};

#endif /* PNDLMDFITFACADE_H_ */
