/*
 * BuilderNode2.cpp
 *
 *  Created on: Oct 30, 2013
 *      Author: Jonas Kunze
 */

#include "BuilderNode.h"

BuilderNode::BuilderNode(u_int32_t _deltaScore, std::string _suffix) :
		deltaScore(_deltaScore), suffix(_suffix) {

}

BuilderNode::~BuilderNode() {
}

bool BuilderNodeComparator::operator()(
		const std::shared_ptr<BuilderNode>& left,
		const std::shared_ptr<BuilderNode>& right) {
	return left->deltaScore < right->deltaScore;
}
