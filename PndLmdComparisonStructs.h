/*
 * PndLmdComparisonStructs.h
 *
 *  Created on: Jul 14, 2014
 *      Author: steve
 */

#ifndef PNDLMDCOMPARISONSTRUCTS_H_
#define PNDLMDCOMPARISONSTRUCTS_H_

#include "LumiFitStructs.h"
#include "fit/PndLmdFitOptions.h"
#include "data/PndLmdAbstractData.h"

namespace LumiFit {

  namespace Comparisons {

    class AbstractLmdDataFilter {
    public:
      virtual ~AbstractLmdDataFilter() {
      }
      virtual bool check(const PndLmdAbstractData &lmd_abs_data) =0;
    };

// compare estimator options only
    struct FitOptionsCompare {
      bool operator()(const PndLmdFitOptions &lhs,
          const PndLmdFitOptions &rhs) {
        if (lhs.getEstimatorOptions() < rhs.getEstimatorOptions())
          return true;
        else if (lhs.getEstimatorOptions() > rhs.getEstimatorOptions())
          return false;
        return false;
      }
    };

    struct DataPrimaryDimensionOptionsFilter: public AbstractLmdDataFilter {
      LumiFit::LmdDimensionOptions lmd_dim_opt;

      DataPrimaryDimensionOptionsFilter(
          LumiFit::LmdDimensionOptions lmd_dim_opt_) :
          lmd_dim_opt(lmd_dim_opt_) {
      }
      bool check(const PndLmdAbstractData &lmd_abs_data) {
        return (lmd_abs_data.getPrimaryDimension().dimension_options
            == lmd_dim_opt);
      }

      void setDimensionOptions(
          const LumiFit::LmdDimensionOptions &lmd_dim_opt_) {
        lmd_dim_opt = lmd_dim_opt_;
      }
    };

    struct DataPrimaryDimensionTrackTypeFilter: public AbstractLmdDataFilter {
      LumiFit::LmdTrackType lmd_track_type;

      DataPrimaryDimensionTrackTypeFilter(LumiFit::LmdTrackType lmd_track_type_) :
          lmd_track_type(lmd_track_type_) {
      }
      bool check(const PndLmdAbstractData &lmd_abs_data) {
        return (lmd_abs_data.getPrimaryDimension().dimension_options.track_type
            == lmd_track_type);
      }
    };

    struct SelectionDimensionFilter: public AbstractLmdDataFilter {
      LumiFit::LmdDimensionOptions lmd_dim_opt;

      SelectionDimensionFilter(LumiFit::LmdDimensionOptions lmd_dim_opt_) :
          lmd_dim_opt(lmd_dim_opt_) {
      }
      bool check(const PndLmdAbstractData &lmd_abs_data) {
        const std::set<LumiFit::LmdDimension> &selection_dimensions(
            lmd_abs_data.getSelectorSet());
        std::set<LumiFit::LmdDimension>::const_iterator sel_dim_it;
        bool found(false);
        for (sel_dim_it = selection_dimensions.begin();
            sel_dim_it != selection_dimensions.end(); ++sel_dim_it) {
          if (sel_dim_it->dimension_options == lmd_dim_opt) {
            found = true;
            break;
          }
        }
        return found;
      }
    };

    struct NegatedSelectionDimensionFilter: public AbstractLmdDataFilter {
      LumiFit::LmdDimensionOptions lmd_dim_opt;

      NegatedSelectionDimensionFilter(LumiFit::LmdDimensionOptions lmd_dim_opt_) :
          lmd_dim_opt(lmd_dim_opt_) {
      }
      bool check(const PndLmdAbstractData &lmd_abs_data) {
        const std::set<LumiFit::LmdDimension> &selection_dimensions(
            lmd_abs_data.getSelectorSet());
        std::set<LumiFit::LmdDimension>::const_iterator sel_dim_it;
        bool found(false);
        for (sel_dim_it = selection_dimensions.begin();
            sel_dim_it != selection_dimensions.end(); ++sel_dim_it) {
          if (sel_dim_it->dimension_options == lmd_dim_opt) {
            found = true;
          }
        }
        return !found;
      }
    };

    struct SelectionDimensionsFilterIgnoringSecondaryTrack: public AbstractLmdDataFilter {
      SelectionDimensionsFilterIgnoringSecondaryTrack(
          const std::set<LumiFit::LmdDimension> &selection_dimensions_) {
        selection_dimensions = removeSecondaryTrackFilter(
            selection_dimensions_);
      }
      bool check(const PndLmdAbstractData &lmd_abs_data) {
        std::set<LumiFit::LmdDimension> filtered_selection_dimensions =
            removeSecondaryTrackFilter(lmd_abs_data.getSelectorSet());
        std::cout << "checking if " << filtered_selection_dimensions.size()
            << " and " << selection_dimensions.size() << " match" << std::endl;
        return filtered_selection_dimensions == selection_dimensions;
      }
      static std::set<LumiFit::LmdDimension> removeSecondaryTrackFilter(
          std::set<LumiFit::LmdDimension> selection_dimensions_) {
        std::set<LumiFit::LmdDimension>::iterator it =
            selection_dimensions_.begin();
        while (it != selection_dimensions_.end()) {
          if (it->dimension_options.dimension_type == LumiFit::SECONDARY) {
            selection_dimensions_.erase(it);
            it = selection_dimensions_.begin();
          } else
            ++it;
        }
        return selection_dimensions_;
      }

    private:
      std::set<LumiFit::LmdDimension> selection_dimensions;
    };

    struct SecondaryTrackFilter: public AbstractLmdDataFilter {
      bool check(const PndLmdAbstractData &lmd_abs_data) {
        for (auto const& sel_dim : lmd_abs_data.getSelectorSet()) {
          if (sel_dim.dimension_options.dimension_type == LumiFit::SECONDARY) {
            return true;
          }
        }
        return false;
      }
    };

    struct NoSecondaryTrackFilter: public AbstractLmdDataFilter {
      bool check(const PndLmdAbstractData &lmd_abs_data) {
        for (auto const& sel_dim : lmd_abs_data.getSelectorSet()) {
          if (sel_dim.dimension_options.dimension_type == LumiFit::SECONDARY) {
            return false;
          }
        }
        return true;
      }
    };
  }
}

#endif /* PNDLMDCOMPARISONSTRUCTS_H_ */
