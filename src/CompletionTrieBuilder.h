/*
 * CompletionTrieBuilder.h
 *
 *  Created on: Oct 28, 2013
 *      Author: Jonas Kunze
 */

#ifndef COMPLETIONTRIEBUILDER_H_
#define COMPLETIONTRIEBUILDER_H_

#include <deque>
#include <set>
#include <string>

#include "BuilderNode.h"

class CompletionTrie;

class CompletionTrieBuilder {
public:
	CompletionTrieBuilder();
	virtual ~CompletionTrieBuilder();

	void addString(std::string str, u_int32_t score);
	CompletionTrie generateCompletionTrie();

private:
	std::shared_ptr<BuilderNode> root;
	std::set<BuilderNode, BuilderNode> nodesSortedByScore;

	std::deque<std::shared_ptr<BuilderNode> > findLocus(std::string term,
			bool& return_foundTerm);
};

#endif /* COMPLETIONTRIEBUILDER_H_ */
