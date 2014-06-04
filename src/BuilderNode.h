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
	u_int32_t score_;
	BuilderNode* parent;
	bool isLastSibbling_;

	std::string suffix_;
	std::set<BuilderNode*, BuilderNodeComparator> children;
	u_int16_t trieLayer;

	/*
	 * The absolute pointer to the firstChild. Use this while writing this node to calculate the offset
	 */
	u_int32_t firstChildPointer_;

	std::string additionalData_;

	BuilderNode(BuilderNode* parent, u_int32_t score, std::string _suffix);
	BuilderNode() :
			score_(0), parent(nullptr), isLastSibbling_(false), trieLayer(0), firstChildPointer_(
					0), additionalData_(""){
	}
	virtual ~BuilderNode();

	void addChild(BuilderNode* child);

	u_int32_t calculatePackedNodeSize(u_int32_t nodePointer) {
		return PackedNode::calculateSize(suffix_.length(), getDeltaScore(),
				firstChildPointer_ != 0 ? firstChildPointer_ - nodePointer : 0);
	}

	inline u_int32_t getDeltaScore() const {
		return parent != nullptr ? parent->score_ - score_ : 0xFFFFFFFF - score_;
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

	void setParent(BuilderNode* parent) {
		this->parent = parent;

		if (parent != nullptr) {
			if (score_ > parent->score_) {
				parent->updateChildScore(score_);
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

	inline void setAdditionalData(std::string image) {
		this->additionalData_ = image;
	}


	void updateChildScore(u_int32_t childScore) {
		if (childScore >score_) {
			score_ = childScore;
			if(parent != nullptr){
				parent->updateChildScore(childScore);
			}
		}
	}
};

#endif /* BUILDERNODE2_H_ */
