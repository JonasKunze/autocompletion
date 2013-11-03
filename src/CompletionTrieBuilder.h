/*
 * CompletionTrieBuilder.h
 *
 *  Created on: Oct 28, 2013
 *      Author: Jonas Kunze
 */

#ifndef COMPLETIONTRIEBUILDER_H_
#define COMPLETIONTRIEBUILDER_H_

#include <stack>
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

	/**
	 * Creates a list of nodes defining the longest maximum substr of term. Nodes returned
	 * will never be leaf nodes with only one character
	 *
	 * @param numberOfCharsFound The number of characters that fit term. If it equals term.length() the whole term is
	 * encoded in the returned list of nodes. Else term.substr(numberOfCharsFound) is the remaining suffix that needs
	 * to be stored
	 *
	 * @charsRemainingForLastNode The number of characters of the last node that do not fit term
	 */
	std::stack<BuilderNode*> findLocus(std::string term,
			unsigned short& numberOfCharsFound,
			unsigned char& charsRemainingForLastNode);
	void splitNode(BuilderNode* node, unsigned char splitPos);

};

#endif /* COMPLETIONTRIEBUILDER_H_ */
