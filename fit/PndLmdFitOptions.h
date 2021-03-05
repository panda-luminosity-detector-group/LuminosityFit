#ifndef PNDLMDFITOPTIONS_H_
#define PNDLMDFITOPTIONS_H_

#include "LumiFitStructs.h"
#ifndef __CINT__
#include "core/ModelStructs.h"
#include "fit/EstimatorOptions.h"
#endif /* __CINT __ */

#include <map>
#include <set>
#include <string>

#include "TObject.h"

#ifndef __CINT__
#include <boost/property_tree/ptree_fwd.hpp>
#endif /* __CINT __ */

/**
 * \brief This class contains the various fit options which are independent of
 * the fit model itself.
 *
 * The user has the choice between several different fit options
 * - model options
 * - fit ranges
 * - estimator type
 * - estimator options
 *
 * The fields can only be set by the user within the #PndLmdFitFacade class, to
 * avoid further alterations of the variables after a their initial setting.
 */
class PndLmdFitOptions : public TObject {
  friend class PndLmdFitFacade;

private:
  // usually we would just have the ptree here
  // unfortunately root cint is not capable to parse the ptree header
  // so this simpler option map was "developed"...
  std::map<std::string, std::string> model_opt_map;

#ifndef __CINT__
  EstimatorOptions est_opt;
#endif /* __CINT __ */

  LumiFit::LmdEstimatorType estimator_type;

  std::set<std::string, ModelStructs::string_comp> free_parameter_names;

public:
  PndLmdFitOptions();

#ifndef __CINT__
  boost::property_tree::ptree getModelOptionsPropertyTree() const;
#endif /* __CINT __ */

#ifndef __CINT__
  const EstimatorOptions &getEstimatorOptions() const;
#endif /* __CINT __ */
  const std::set<std::string, ModelStructs::string_comp> &
  getFreeParameterSet() const;

  bool lessThanModelOptionsProperyTree(
      const std::map<std::string, std::string> &lhs,
      const std::map<std::string, std::string> &rhs) const;

  /**
   * Less then operator. Will return true if "this" fit options are "less" in
   * value.
   */
  bool operator<(const PndLmdFitOptions &rhs) const;
  /**
   * Greater then operator. Will return true if "this" fit options are "greater"
   * in value.
   */
  bool operator>(const PndLmdFitOptions &rhs) const;
  /**
   * Comparison operator. Will return true only if all fit options are equal in
   * value.
   */
  bool operator==(const PndLmdFitOptions &fit_options) const;
  /**
   * Inverse comparison operator @see operator==()
   */
  bool operator!=(const PndLmdFitOptions &fit_options) const;
  /**
   * Output stream operator for printing out fit options for information.
   */
  friend std::ostream &operator<<(std::ostream &os,
                                  const PndLmdFitOptions &fit_options);

  ClassDef(PndLmdFitOptions, 1);
};

#endif /* PNDLMDFITOPTIONS_H_ */
