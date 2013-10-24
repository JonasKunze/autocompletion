/*
 * CompletionTrie.cpp
 *
 *  Created on: Oct 23, 2013
 *      Author: Jonas Kunze
 */

#include "CompletionTrie.h"

//#include <stdlib.h>
#include <iostream>

#include "PackedNode.h"

static u_int64_t characterMask[] = { 0, 0xFF, 0xFFFF, 0xFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFFFF, 0xFFFFFFFFFFFF, 0xFFFFFFFFFFFFFF };
CompletionTrie::CompletionTrie() :
		mem(new char[initialMemSize]), memSize(initialMemSize), firstNodePointer(
				0), lastNodePointer(0) {

}

CompletionTrie::~CompletionTrie() {
	delete[] mem;
}

/**
 *
 */
void CompletionTrie::addTerm(const std::string term, const u_int32_t score) {
	if (firstNodePointer == 0) {
		firstNodePointer = memSize / 2;
		lastNodePointer = firstNodePointer;

		PackedNode::createNode(mem, firstNodePointer, term.length(),
				term.c_str(), true, 1000, 0, false);
	} else {
		bool foundCompleteTerm = false;
		std::vector<PackedNode*> locus = findLocus(term, foundCompleteTerm);
		PackedNode* closestNode = locus.back();

		std::string completeString = "";
		for (PackedNode* n : locus) {
			completeString += std::string(n->getCharacters(),
					n->charactersSize_);
		}
		std::cout << completeString << std::endl;

		if (foundCompleteTerm) {
			// Term already exists!
			return;
		}
	}
}

/**
 * Returns the node defining as many characters of {term} as possible. return_foundTerm will be set to
 * tru if the whole term is defined by the returned node
 *
 * @param term The string searched for
 * @param return_foundTerm true if the whole term is defined by the returned node
 *
 * @return The Node in the trie defining the maximum largest substring of term or the whole term
 */
std::vector<PackedNode*> CompletionTrie::findLocus(const std::string term,
		bool& return_foundTerm) {
	std::vector<PackedNode*> resultLocus;

	return_foundTerm = false;
	uint charPos = 0;
	const char* prefixChars = term.c_str();
	u_int64_t nodePointer = firstNodePointer;
	u_int64_t firstNonSiblingPointer = 0;

	u_int64_t currentNode;
	PackedNode* n;
	do {
		currentNode = *((u_int64_t*) (mem + nodePointer));
		n = (PackedNode*) (&currentNode);

		/*
		 * Try if the N characters in the node match the next N characters of the prefix
		 */
		bool nodeFits = (currentNode >> 8 & characterMask[n->charactersSize_])
				== (*((u_int64_t*) (prefixChars + charPos))
						& characterMask[n->charactersSize_]);
		if (nodeFits) {
			resultLocus.push_back(n);
			firstNonSiblingPointer = 0;
			std::cout << std::string(n->getCharacters(), n->charactersSize_)
					<< " : " << nodeFits << std::endl;
			charPos += n->charactersSize_;

			/*
			 * Is
			 */
			if (n->firstChildOffsetSize_ == 0) {
				// No more children
				if (charPos == term.size() && n->isEndOfWord_) {
					// we've reached the end of the term
					return_foundTerm = true;
					return resultLocus;
				} else {
					// this node defines only a part of the term but we've reached the end
					// It's like searching for "autocompletion" but only "auto" exists
					return resultLocus;
				}
			}
			nodePointer += n->getFirstChildOffset();
		} else {
			if (firstNonSiblingPointer == 0) {
				/*
				 * nodeFits is false for the first time at the current depth
				 * Save the pointer of the first child as iterating through all siblings must be stopped there
				 */
				firstNonSiblingPointer = nodePointer + n->getFirstChildOffset();
			}
			/*
			 * Move to the next sibling
			 */
			nodePointer += n->getSize();
		}

	} while (firstNonSiblingPointer == 0 || nodePointer < firstNonSiblingPointer);

	return resultLocus;
}
