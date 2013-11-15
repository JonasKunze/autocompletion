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
private:
	BuilderNode* parent;

public:
	bool isLastSibbling;
	u_int32_t score;
	u_int32_t parentScore;
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
			parent(nullptr), isLastSibbling(false), score(0), parentScore(0), trieLayer(
					0), firstChildPointer(0), URI(""), image("") {
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

	inline BuilderNode* getParent() const {
		return parent;
	}

	/**
	 * This is called very often therefore parentScore is cached
	 */
	inline u_int32_t getParentScore() const {
		return parentScore;
//		return parent == nullptr ? 0xFFFFFFFF : parent->score;
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

	std::string getImage() const {
		return image;
	}

	void setImage(std::string image) {
		this->image = image;
	}

	std::string getURI() const {
		return URI;
	}

	void setURI(std::string URI) {
		this->URI = URI;
	}
};

#endif /* BUILDERNODE2_H_ */
