/*
 * BuilderNode2.cpp
 *
 *  Created on: Oct 30, 2013
 *      Author: Jonas Kunze
 */

#include "BuilderNode.h"

#include <iostream>

std::set<std::shared_ptr<BuilderNode>, BuilderNodeComparator> BuilderNode::allNodes;

bool BuilderNodeComparator::operator()(const std::shared_ptr<BuilderNode>& left,
		const std::shared_ptr<BuilderNode>& right) {
	return left->deltaScore < right->deltaScore;
}

bool BuilderNodeLayerComparator::operator()(
		const std::shared_ptr<BuilderNode>& left,
		const std::shared_ptr<BuilderNode>& right) {
	if (left->trieLayer != right->trieLayer) {
		return left->trieLayer < right->trieLayer;
	}
	return left->deltaScore < right->deltaScore;
}

BuilderNode::BuilderNode(std::shared_ptr<BuilderNode> _parent,
		u_int32_t _deltaScore, const std::string _suffix) :
		parent(_parent), trieLayer(
				parent.get() == NULL ? 0 : parent->trieLayer + 1), deltaScore(
				_deltaScore), suffix(_suffix) {
	allNodes.insert(std::shared_ptr<BuilderNode>(this));
	std::cout << suffix.size() << "?!?!" << std::endl;
}

BuilderNode::~BuilderNode() {
}

void BuilderNode::addChild(std::shared_ptr<BuilderNode> child) {
	children.insert(child);
}

