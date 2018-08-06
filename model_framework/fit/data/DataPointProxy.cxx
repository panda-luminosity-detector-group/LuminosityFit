/*
 * DataPointProxy.cxx
 *
 *  Created on: Jun 16, 2013
 *      Author: steve
 */

#include "DataPointProxy.h"

DataPointProxy::DataPointProxy() : state(-1),  is_point_used(true) {
}

DataPointProxy::~DataPointProxy() {
}

bool DataPointProxy::isBinnedDataPoint() const {
	return (0 == state);
}
bool DataPointProxy::isUnbinnedDataPoint() const {
	return (1 == state);
}

const std::shared_ptr<DataStructs::binned_data_point> DataPointProxy::getBinnedDataPoint() const {
	return bdp;
}

const std::shared_ptr<DataStructs::unbinned_data_point> DataPointProxy::getUnbinnedDataPoint() const {
	return udp;
}

void DataPointProxy::setBinnedDataPoint(std::shared_ptr<DataStructs::binned_data_point> bdp_) {
	state = 0;
	udp.reset();
	bdp = bdp_;
}

void DataPointProxy::setUnbinnedDataPoint(std::shared_ptr<DataStructs::unbinned_data_point> udp_) {
	state = 1;
	bdp.reset();
	udp = udp_;
}

bool DataPointProxy::isPointUsed() const
{
    return is_point_used;
}

void DataPointProxy::setPointUsed(bool is_point_used_)
{
    is_point_used = is_point_used_;
}
