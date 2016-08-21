#include "PndLmdMapData.h"

ClassImp(PndLmdMapData);

PndLmdMapData::PndLmdMapData() :
    data_tree(0) {

}
PndLmdMapData::PndLmdMapData(const PndLmdMapData &lmd_hist_data_) :
    PndLmdAbstractData(lmd_hist_data_), hit_map_2d(lmd_hist_data_.hit_map_2d) {
  if (lmd_hist_data_.data_tree) {
    data_tree = (TTree*) lmd_hist_data_.data_tree->Clone();
    data_tree->SetDirectory(0);
  }
}

PndLmdMapData::~PndLmdMapData() {
  if (data_tree) {
    delete data_tree;
  }
}

void PndLmdMapData::init1DData() {

}
void PndLmdMapData::init2DData() {

}

const std::map<Point2D, Point2DCloud>& PndLmdMapData::getHitMap() const {
  return hit_map_2d;
}

void PndLmdMapData::add(const PndLmdAbstractData &lmd_abs_data_addition) {
  const PndLmdMapData * lmd_data_addition =
      dynamic_cast<const PndLmdMapData*>(&lmd_abs_data_addition);
  if (lmd_data_addition) {
    if (getPrimaryDimension().dimension_range
        == lmd_data_addition->getPrimaryDimension().dimension_range) {
      setNumEvents(getNumEvents() + lmd_data_addition->getNumEvents());
      if (getSecondaryDimension().is_active) {
        if (getSecondaryDimension().dimension_range
            == lmd_data_addition->getSecondaryDimension().dimension_range) {
          for (auto const& entry : lmd_data_addition->getHitMap()) {
            hit_map_2d[entry.first].total_count += entry.second.total_count;
            for (auto const& reco_bin : entry.second.points) {
              hit_map_2d[entry.first].points[reco_bin.first] += reco_bin.second;
            }
          }
        }
      }
    }
  }
}

// histogram filling methods
void PndLmdMapData::addData(const std::vector<double> &values) {
  Point2D mc_point(values[2], values[3]);
  Point2D rec_point(values[0], values[1]);
  ++(hit_map_2d[mc_point].points[rec_point]);
  ++(hit_map_2d[mc_point].total_count);
}

PndLmdMapData& PndLmdMapData::operator=(const PndLmdMapData &lmd_hist_data) {
  PndLmdAbstractData::operator=(lmd_hist_data);

  hit_map_2d = lmd_hist_data.hit_map_2d;
  if (lmd_hist_data.data_tree) {
    data_tree = lmd_hist_data.data_tree->CloneTree();
    data_tree->SetDirectory(0);
  }

  return (*this);
}

void PndLmdMapData::clearMap() {
  hit_map_2d.clear();
}

void PndLmdMapData::saveToRootFile() {
  std::cout << "Saving " << getName() << " to current root file..."
      << std::endl;
  convertToRootTree();
  this->Write(getName().c_str());
}

void PndLmdMapData::convertToRootTree() {
  if (data_tree)
    delete data_tree;

  data_tree = new TTree();

  const Point2D *mc_point;
  const Point2DCloud *reco_points;

  data_tree->SetBranchAddress("mc_point", &mc_point);
  data_tree->SetBranchAddress("reco_points", &reco_points);

  for (auto const& entry : hit_map_2d) {
    mc_point = &entry.first;
    reco_points = &entry.second;
    data_tree->Fill();
  }
}
void PndLmdMapData::convertFromRootTree() {
  if (data_tree) {
    Point2D mc_point;
    Point2D *pmc_point(&mc_point);
    Point2DCloud reco_points;
    Point2DCloud *preco_points(&reco_points);

    data_tree->SetBranchAddress("mc_point", &pmc_point);
    data_tree->SetBranchAddress("reco_points", &preco_points);

    for (unsigned int i = 0; i < data_tree->GetEntries(); ++i) {
      data_tree->GetEntry(i);
      hit_map_2d[mc_point] = reco_points;
    }
  }
}

