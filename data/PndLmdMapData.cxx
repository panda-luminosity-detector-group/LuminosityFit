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
            auto& hit_map_entry = hit_map_2d[entry.first];
            hit_map_entry.total_count += entry.second.total_count;
            for (auto const& reco_bin : entry.second.points) {
              hit_map_entry.points[reco_bin.first] += reco_bin.second;
            }
          }
        }
      }
    }
  }
}

// histogram filling methods
void PndLmdMapData::addData(const std::vector<double> &values) {
  int mc_idx(
      (values[2] - primary_dimension.dimension_range.getRangeLow())
          / primary_dimension.bin_size);
  int mc_idy(
      (values[3] - secondary_dimension.dimension_range.getRangeLow())
          / secondary_dimension.bin_size);
  Point2D mc_point(
      primary_dimension.dimension_range.getRangeLow()
          + (0.5 + mc_idx) * primary_dimension.bin_size,
      secondary_dimension.dimension_range.getRangeLow()
          + (0.5 + mc_idy) * secondary_dimension.bin_size);
  auto& mc_bin = hit_map_2d[mc_point];
  ++(mc_bin.total_count);
  int rec_idx(
      (values[0] - primary_dimension.dimension_range.getRangeLow())
          / primary_dimension.bin_size);
  int rec_idy(
      (values[1] - secondary_dimension.dimension_range.getRangeLow())
          / secondary_dimension.bin_size);
  Point2D rec_point(
      primary_dimension.dimension_range.getRangeLow()
          + (0.5 + rec_idx) * primary_dimension.bin_size,
      secondary_dimension.dimension_range.getRangeLow()
          + (0.5 + rec_idy) * secondary_dimension.bin_size);
  ++(mc_bin.points[rec_point]);
}

PndLmdMapData& PndLmdMapData::operator=(const PndLmdMapData &lmd_hist_data) {
  PndLmdAbstractData::operator=(lmd_hist_data);
  hit_map_2d = lmd_hist_data.hit_map_2d;
  if (lmd_hist_data.data_tree) {
    data_tree = (TTree*) lmd_hist_data.data_tree->Clone();
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
  std::cout << "Conversion done! Writing to file..." << std::endl;
  this->Write(getName().c_str());
  std::cout << "finished!" << std::endl;
}

void PndLmdMapData::convertToRootTree() {
  if (!data_tree)
    delete data_tree;

  std::cout<<"estimating size of data in memory...\n";
  unsigned long bytes_unsigned_ints(0);
  unsigned long bytes_doubles(0);
  unsigned long total_overhead_bytes_unsigned_ints(0);
  unsigned long total_overhead_bytes_doubles(0);
  unsigned short overhead_unsigned_ints(sizeof(std::vector<unsigned int>));
  unsigned short overhead_doubles(sizeof(std::vector<double>));
  unsigned long all_reco_points(0);

  for (auto const& entry : hit_map_2d) {
    bytes_unsigned_ints += overhead_unsigned_ints;
    total_overhead_bytes_unsigned_ints += overhead_unsigned_ints;
    bytes_doubles += overhead_doubles;
    total_overhead_bytes_doubles += overhead_doubles;
    bytes_unsigned_ints += sizeof(unsigned int)*entry.second.points.size();
    bytes_doubles += sizeof(double)*entry.second.points.size();
    all_reco_points += entry.second.points.size();
  }
  std::cout<<"memory summary:\n";
  std::cout<<"overhead for a single unsigned int vector: "<<overhead_unsigned_ints<<" bytes\n";
  std::cout<<"overhead for a single double vector: "<<overhead_doubles<<" bytes\n";
  std::cout<<"number of entries: "<<hit_map_2d.size()<<std::endl;
  std::cout<<"number of reco entries: "<<all_reco_points<<std::endl;
  std::cout<<"total memory consumption for all unsigned int vectors: "<<bytes_unsigned_ints<<" bytes\n";
  std::cout<<"total memory consumption for all double vectors: "<<bytes_doubles<<" bytes\n";
  std::cout<<"total overhead for the unsigned int vectors: "<<total_overhead_bytes_unsigned_ints<<" bytes\n";
  std::cout<<"total overhead for the double vectors: "<<total_overhead_bytes_doubles<<" bytes\n\n";
  std::cout << "converting hit map to root tree...\n";
  data_tree = new TTree();

  // new format
  Point2D mc_point;
  const Point2D *pmc_point(&mc_point);
  std::vector<double> reco_points_x;
  const std::vector<double> *preco_points_x(&reco_points_x);
  std::vector<double> reco_points_y;
  const std::vector<double> *preco_points_y(&reco_points_y);
  std::vector<unsigned int> reco_points_count;
  const std::vector<unsigned int> *preco_points_count(&reco_points_count);
  ULong64_t total_count;

  data_tree->Branch("mc_point", &pmc_point);
  data_tree->Branch("reco_points_x", &preco_points_x);
  data_tree->Branch("reco_points_y", &preco_points_y);
  data_tree->Branch("reco_points_count", &preco_points_count);
  data_tree->Branch("total_count", &total_count, "total_count/l");

  for (auto const& entry : hit_map_2d) {
    pmc_point = &entry.first;
    total_count = entry.second.total_count;
    reco_points_x.clear();
    reco_points_y.clear();
    reco_points_count.clear();
    for (auto const& reco_point : entry.second.points) {
      reco_points_x.push_back(reco_point.first.x);
      reco_points_y.push_back(reco_point.first.y);
      reco_points_count.push_back(reco_point.second);
    }
    data_tree->Fill();
  }
}

void PndLmdMapData::convertFromRootTree() {
  if (data_tree) {
    std::cout << "converting root tree to hit map...\n";

    // check if we still have the old format
    // get branch names
    std::vector<std::string> branch_names;
    TObjArray *branch_list = (TObjArray*)data_tree->GetListOfBranches()->Clone();
    branch_list->SetOwner(kFALSE);
    for (int i = 0; i < branch_list->GetEntries(); ++i) {
      branch_names.push_back(branch_list->At(i)->GetName());
    }

    auto result = std::find_if(branch_names.begin(), branch_names.end(),
        [] (const std::string& s) {return s.find("reco_points_x") != std::string::npos;});

    if (result == branch_names.end()) {
      // old format
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
    } else {
      // new format
      Point2D mc_point;
      Point2D *pmc_point(&mc_point);
      std::vector<double> reco_points_x;
      std::vector<double> *preco_points_x(&reco_points_x);
      std::vector<double> reco_points_y;
      std::vector<double> *preco_points_y(&reco_points_y);
      std::vector<unsigned int> reco_points_count;
      std::vector<unsigned int> *preco_points_count(&reco_points_count);
      ULong64_t total_count;

      data_tree->SetBranchAddress("mc_point", &pmc_point);
      data_tree->SetBranchAddress("reco_points_x", &preco_points_x);
      data_tree->SetBranchAddress("reco_points_y", &preco_points_y);
      data_tree->SetBranchAddress("reco_points_count", &preco_points_count);
      data_tree->SetBranchAddress("total_count", &total_count);

      for (unsigned int i = 0; i < data_tree->GetEntries(); ++i) {
        data_tree->GetEntry(i);
        Point2DCloud reco_points;
        for (unsigned int j = 0; j < reco_points_count.size(); ++j) {
          reco_points.points[Point2D(reco_points_x[j], reco_points_y[j])] =
              reco_points_count[j];
        }
        reco_points.total_count = total_count;
        hit_map_2d[mc_point] = reco_points;
      }
    }
  }
}

