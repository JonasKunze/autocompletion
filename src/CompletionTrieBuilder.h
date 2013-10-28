/*
 * CompletionTrieBuilder.h
 *
 *  Created on: Oct 28, 2013
 *      Author: Jonas Kunze
 */

#ifndef COMPLETIONTRIEBUILDER_H_
#define COMPLETIONTRIEBUILDER_H_

#include <sys/types.h>
#include <map>
#include <string>
#include <vector>
#include <set>

struct BuilderNodeScoreComparator {
	inline bool operator()(const BuilderNode& left, const BuilderNode& right) {
		return left.deltaScore < right.deltaScore;
	}
};

struct BuilderNode {
	u_int32_t deltaScore;
	std::string suffix;
	std::set<BuilderNode, BuilderNodeScoreComparator> children;

	BuilderNode(u_int32_t _deltaScore, std::string _suffix) :
			deltaScore(_deltaScore), suffix(_suffix) {

	}
};

class CompletionTrieBuilder {
public:
	CompletionTrieBuilder();
	virtual ~CompletionTrieBuilder();

	void addString(std::string str);
	char* generateCompletionTrie();

private:
	std::set<BuilderNode, BuilderNodeScoreComparator> nodesSortedByScore;
};

#endif /* COMPLETIONTRIEBUILDER_H_ */
