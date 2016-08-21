#ifndef PNDLMDMAPDATA_H_
#define PNDLMDMAPDATA_H_

#include "PndLmdAbstractData.h"
#include "fit/PndLmdFitStorage.h"

#include "TTree.h"

struct Point2D {
  double x;
  double y;

  Point2D() : x(0.0), y(0.0) {}
  Point2D(double x_, double y_) : x(x_), y(y_) {}

  bool operator<(const Point2D &rhs) const {
    if(x < rhs.x)
      return true;
    else if(x > rhs.x)
      return false;
    if(y < rhs.y)
      return true;
    else if(y > rhs.y)
      return false;
    return false;
  }
};

struct Point2DCloud {
  std::map<Point2D, unsigned int> points;
  unsigned int total_count;
};

class PndLmdMapData: public PndLmdAbstractData  {
#ifndef __CINT__
  std::map<Point2D, Point2DCloud> hit_map_2d;
#endif

  TTree *data_tree;

	void init1DData();
	void init2DData();

public:
	PndLmdMapData();
	PndLmdMapData(const PndLmdMapData &lmd_hist_data_);
	virtual ~PndLmdMapData();

#ifndef __CINT__
	const std::map<Point2D, Point2DCloud>& getHitMap() const;
#endif

	void add(const PndLmdAbstractData &lmd_abs_data_addition);

	// histogram filling methods
	virtual void addData(const std::vector<double> &values);

	PndLmdMapData& operator=(const PndLmdMapData &lmd_hist_data);

	void clearMap();

	void saveToRootFile();

	void convertToRootTree();
	void convertFromRootTree();

	ClassDef(PndLmdMapData, 1);
};

#endif /* PNDLMDMAPDATA_H_ */
