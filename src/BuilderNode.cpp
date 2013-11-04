/*
 * BuilderNode2.cpp
 *
 *  Created on: Oct 30, 2013
 *      Author: Jonas Kunze
 */

#include "BuilderNode.h"

//#include <iostream>

std::vector<BuilderNode*> BuilderNode::allNodes;

bool BuilderNodeComparator::operator()(const BuilderNode* left,
		const BuilderNode* right) {
	if (left->score == right->score) {
		return left->suffix < right->suffix;
	}
	return left->score < right->score;
}

bool BuilderNodeLayerComparator::operator()(const BuilderNode* left,
		const BuilderNode* right) {
	if (left->getTrieLayer() == right->getTrieLayer()) {

		if (left->score == right->score) {
			return left->suffix < right->suffix;
		}
		return left->score < right->score;

	}
	return left->getTrieLayer() < right->getTrieLayer();
}

BuilderNode::BuilderNode(BuilderNode* _parent, u_int32_t _deltaScore,
		const std::string _suffix) :
		parent(_parent), isLastSibbling(false), score(_deltaScore), suffix(
				_suffix), firstChildPointer(0) {
	allNodes.push_back(this);
}

BuilderNode::~BuilderNode() {
}

void BuilderNode::addChild(BuilderNode* child) {
	children.insert(child);
}

