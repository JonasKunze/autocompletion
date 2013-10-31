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
	bool operator()(const std::shared_ptr<BuilderNode>& left,
			const std::shared_ptr<BuilderNode>& right);
};

struct BuilderNodeLayerComparator {
	bool operator()(const std::shared_ptr<BuilderNode>& left,
			const std::shared_ptr<BuilderNode>& right);
};

class BuilderNode {

public:
	std::shared_ptr<BuilderNode> parent;
	u_int16_t trieLayer;
	u_int32_t deltaScore;
	std::string suffix;
	std::set<std::shared_ptr<BuilderNode>, BuilderNodeComparator> children;

	static std::set<std::shared_ptr<BuilderNode>, BuilderNodeComparator> allNodes;

	BuilderNode(std::shared_ptr<BuilderNode> parent, u_int32_t _deltaScore,
			std::string _suffix);
	BuilderNode() :
			trieLayer(0), deltaScore(0) {
	}
	virtual ~BuilderNode();

	void addChild(std::shared_ptr<BuilderNode> child);

};

#endif /* BUILDERNODE2_H_ */
