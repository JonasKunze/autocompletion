/*
 * BuilderNode2.cpp
 *
 *  Created on: Oct 30, 2013
 *      Author: Jonas Kunze
 */

#include "BuilderNode.h"

#include <iostream>

std::set<BuilderNode*, BuilderNodeComparator> BuilderNode::allNodes;

bool BuilderNodeComparator::operator()(const BuilderNode* left,
		const BuilderNode* right) {
	return left->deltaScore < right->deltaScore;
}

bool BuilderNodeLayerComparator::operator()(const BuilderNode*& left,
		const BuilderNode*& right) {
	if (left->trieLayer != right->trieLayer) {
		return left->trieLayer < right->trieLayer;
	}
	return left->deltaScore < right->deltaScore;
}

BuilderNode::BuilderNode(BuilderNode* _parent, u_int32_t _deltaScore,
		const std::string _suffix) :
		parent(_parent), trieLayer(parent == NULL ? 0 : parent->trieLayer + 1), deltaScore(
				_deltaScore), suffix(_suffix) {
	allNodes.insert(this);
	std::cout << suffix << "?!?!" << std::endl;
}

BuilderNode::~BuilderNode() {
}

void BuilderNode::addChild(BuilderNode* child) {
	children.insert(child);
}

