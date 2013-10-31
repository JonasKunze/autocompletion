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
	CompletionTrie* generateCompletionTrie();
	void print();
	void printNode(BuilderNode* node, std::deque<BuilderNode*> locus);
private:
	BuilderNode* root;

	std::deque<BuilderNode*> findLocus(std::string term,
			unsigned short& numberOfCharsFound, u_int32_t& score,
			unsigned char& charsRemainingForLastNode);
	void splitNode(BuilderNode* node, unsigned char splitPos);

};

#endif /* COMPLETIONTRIEBUILDER_H_ */
