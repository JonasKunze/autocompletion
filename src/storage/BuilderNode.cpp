/*
 * BuilderNode2.cpp
 *
 *  Created on: Oct 30, 2013
 *      Author: Jonas Kunze
 */

#include "BuilderNode.h"

bool BuilderNodeComparator::operator()(const BuilderNode* left,
		const BuilderNode* right) {
	if (left->score_ == right->score_) {
		return left->suffix_ < right->suffix_;
	}
	return left->score_ < right->score_;
}

BuilderNode::BuilderNode(BuilderNode* parent, u_int32_t score,
		const std::string _suffix) :
		score_(score), isLastSibbling_(false), suffix_(_suffix), firstChildPointer_(
				0), additionalData_("") {
	setParent(parent);
}

BuilderNode::~BuilderNode() {
}

void BuilderNode::addChild(BuilderNode* child) {
	children.insert(child);
}

