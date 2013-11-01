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

struct BuilderNodeLayerComparator {
	bool operator()(const BuilderNode* left, const BuilderNode* right);
};

class BuilderNode {

public:
	BuilderNode* parent;
	u_int16_t trieLayer;
	bool isLastSibbling;
	u_int32_t score;
	std::string suffix;
	std::set<BuilderNode*, BuilderNodeComparator> children;

	/*
	 * The absolute pointer to the firstChild. Use this while writing this node to calculate the offset
	 */
	u_int32_t firstChildPointer;

	static std::vector<BuilderNode*> allNodes;

	BuilderNode(BuilderNode* parent, u_int32_t score, std::string _suffix);
	BuilderNode() :
			parent(NULL), trieLayer(0), isLastSibbling(false), score(0), firstChildPointer(
					0) {
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
};

#endif /* BUILDERNODE2_H_ */
