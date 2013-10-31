/*
 * BuilderNode2.h
 *
 *  Created on: Oct 30, 2013
 *      Author: Jonas Kunze
 */

#ifndef BUILDERNODE2_H_
#define BUILDERNODE2_H_

#include <sys/types.h>
#include <memory>
#include <set>
#include <string>

class BuilderNode;

struct BuilderNodeComparator {
	bool operator()(const BuilderNode* left, const BuilderNode* right);
};

struct BuilderNodeLayerComparator {
	bool operator()(const BuilderNode*& left, const BuilderNode*& right);
};

class BuilderNode {

public:
	BuilderNode* parent;
	u_int16_t trieLayer;
	u_int32_t deltaScore;
	std::string suffix;
	std::set<BuilderNode*, BuilderNodeComparator> children;

	static std::set<BuilderNode*, BuilderNodeComparator> allNodes;

	BuilderNode(BuilderNode* parent, u_int32_t _deltaScore,
			std::string _suffix);
	BuilderNode() :
			trieLayer(0), deltaScore(0) {
	}
	virtual ~BuilderNode();

	void addChild(BuilderNode* child);

};

#endif /* BUILDERNODE2_H_ */
