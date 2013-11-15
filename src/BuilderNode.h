/*
 * BuilderNode2.h
 *
 *  Created on: Oct 30, 2013
 *      Author: Jonas Kunze
 */

#ifndef BUILDERNODE2_H_
#define BUILDERNODE2_H_

#include <sys/types.h>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "PackedNode.h"

class BuilderNode;

struct BuilderNodeComparator {
	bool operator()(const BuilderNode* left, const BuilderNode* right);
};

class BuilderNode {

public:
	u_int32_t score;
	BuilderNode* parent;
	bool isLastSibbling;

	std::string suffix;
	std::set<BuilderNode*, BuilderNodeComparator> children;
	u_int16_t trieLayer;

	/*
	 * The absolute pointer to the firstChild. Use this while writing this node to calculate the offset
	 */
	u_int32_t firstChildPointer;

	std::string URI;
	std::string image;

	static std::vector<BuilderNode*> allNodes;

	BuilderNode(BuilderNode* parent, u_int32_t score, std::string _suffix);
	BuilderNode() :
			score(0), parent(nullptr), isLastSibbling(false), trieLayer(0), firstChildPointer(
					0), URI(""), image("") {
	}
	virtual ~BuilderNode();

	void addChild(BuilderNode* child);

	u_int32_t calculatePackedNodeSize(u_int32_t nodePointer) {
		return PackedNode::calculateSize(suffix.length(), getDeltaScore(),
				firstChildPointer != 0 ? firstChildPointer - nodePointer : 0);
	}

	inline u_int32_t getDeltaScore() const {
		return parent != nullptr ? parent->score - score : 0xFFFFFFFF - score;
	}

	inline bool isLeafNode() {
		return children.size() == 0;
	}

	inline u_int16_t getTrieLayer() const {
		return trieLayer;
	}

	inline bool isRootNode() const {
		return parent == nullptr;
	}

//	inline BuilderNode* getParent() const {
//		return parent;
//	}

	void setParent(BuilderNode* parent) {
		this->parent = parent;

		if (parent != nullptr) {
			trieLayer = parent->trieLayer + 1;
			if (score > parent->score) {
				parent->score = score;
			}
			trieLayer = parent->trieLayer + 1;
			for (BuilderNode* child : children) {
				child->setTrieLayer(trieLayer + 1);
			}
		} else {
			trieLayer = 0;
		}
	}

	void setTrieLayer(u_int16_t _trieLayer) {
		trieLayer = _trieLayer;
		for (BuilderNode* child : children) {
			child->setTrieLayer(trieLayer + 1);
		}
	}

	inline void setImage(std::string image) {
		this->image = image;
	}

	inline void setURI(std::string URI) {
		this->URI = URI;
	}

	void setScore(u_int32_t score) {
		if (parent != nullptr && score > parent->score) {
			parent->score = score;
		}

		this->score = score;
	}
};

#endif /* BUILDERNODE2_H_ */
