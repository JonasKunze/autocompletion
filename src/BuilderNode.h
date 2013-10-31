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
	bool operator()(const std::shared_ptr<BuilderNode>& left, const std::shared_ptr<BuilderNode>& right);
};

class BuilderNode {
public:
	u_int32_t deltaScore;
	std::string suffix;
	std::set<std::shared_ptr<BuilderNode>, BuilderNodeComparator> children;

	BuilderNode(u_int32_t _deltaScore, std::string _suffix);
	BuilderNode() :
			deltaScore(0) {
	}
	virtual ~BuilderNode();

};

#endif /* BUILDERNODE2_H_ */
