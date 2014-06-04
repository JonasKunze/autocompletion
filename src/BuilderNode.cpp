/*
 * BuilderNode2.cpp
 *
 *  Created on: Oct 30, 2013
 *      Author: Jonas Kunze
 */

#include "BuilderNode.h"

bool BuilderNodeComparator::operator()(const BuilderNode* left,
		const BuilderNode* right) {
	if (left->score == right->score) {
		return left->suffix < right->suffix;
	}
	return left->score < right->score;
}

BuilderNode::BuilderNode(BuilderNode* _parent, u_int32_t _score,
		const std::string _suffix) :
		score(_score), isLastSibbling(false), suffix(_suffix), firstChildPointer(
				0), URI(""), image("") {
	setParent(_parent);
}

BuilderNode::~BuilderNode() {
}

void BuilderNode::addChild(BuilderNode* child) {
	children.insert(child);
}

