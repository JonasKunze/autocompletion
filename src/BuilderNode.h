/*
 * BuilderNode2.h
 *
 *  Created on: Oct 30, 2013
 *      Author: Jonas Kunze
 */

#ifndef BUILDERNODE2_H_
#define BUILDERNODE2_H_

#include <sys/types.h>
#include <set>
#include <string>
#include <vector>

#include "PackedNode.h"

class BuilderNode;

struct BuilderNodeComparator {
	bool operator()(const BuilderNode* left, const BuilderNode* right);
};

class BuilderNode {
private:
	BuilderNode* parent;

public:
	bool isLastSibbling;
	u_int32_t score;
	std::string suffix;
	std::set<BuilderNode*, BuilderNodeComparator> children;
	u_int16_t trieLayer;

	/*
	 * The absolute pointer to the firstChild. Use this while writing this node to calculate the offset
	 */
	u_int32_t firstChildPointer;

	static std::vector<BuilderNode*> allNodes;

	BuilderNode(BuilderNode* parent, u_int32_t score, std::string _suffix);
	BuilderNode() :
			parent(nullptr), isLastSibbling(false), score(0), firstChildPointer(
					0), trieLayer(0) {
	}
	virtual ~BuilderNode();

	void addChild(BuilderNode* child);

	u_int8_t calculatePackedNodeSize(u_int32_t nodePointer) {
		return PackedNode::calculateSize(suffix.length(), getDeltaScore(),
				firstChildPointer != 0 ? firstChildPointer - nodePointer : 0);
	}

	inline u_int32_t getDeltaScore() const {
		return parent != 0 ? score - parent->score : 0;
	}

	bool isLeafNode() {
		return children.size() == 0;
	}

	u_int16_t getTrieLayer() const {
		return trieLayer;
	}

	bool isRootNode() const {
		return parent == nullptr;
	}

	BuilderNode* getParent() const {
		return parent;
	}

	void setParent(BuilderNode* parent) {
		this->parent = parent;
		trieLayer = parent->trieLayer + 1;
		for (BuilderNode* child : children) {
			child->setTrieLayer(trieLayer + 1);
		}
	}

	void setTrieLayer(u_int16_t _trieLayer) {
		trieLayer = _trieLayer;
		for (BuilderNode* child : children) {
			child->setTrieLayer(trieLayer + 1);
		}
	}
};

#endif /* BUILDERNODE2_H_ */
