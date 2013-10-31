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
	void print();
	void printNode(std::shared_ptr<BuilderNode>,
			std::deque<std::shared_ptr<BuilderNode> > locus);
private:
	std::shared_ptr<BuilderNode> root;

	std::deque<std::shared_ptr<BuilderNode> > findLocus(std::string term,
			unsigned short& numberOfCharsFound, u_int32_t& score,
			unsigned char& charsRemainingForLastNode);
	void splitNode(std::shared_ptr<BuilderNode> node, unsigned char splitPos);

};

#endif /* COMPLETIONTRIEBUILDER_H_ */
