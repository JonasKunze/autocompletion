/*
 * CompletionTrie.cpp
 *
 *  Created on: Oct 23, 2013
 *      Author: Jonas Kunze
 */

#include "CompletionTrie.h"

#include <sys/types.h>
#include <cstring>
#include <iostream>
#include <memory>
#include <new>
#include <string>
#include <vector>

#include "PackedNode.h"

static u_int64_t characterMask[] = { 0, 0xFF, 0xFFFF, 0xFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFFFF, 0xFFFFFFFFFFFF, 0xFFFFFFFFFFFFFF };
CompletionTrie::CompletionTrie() :
		mem(new char[initialMemSize]), memSize(initialMemSize), firstNodePointer(
				0), firstFreeMemPointer(0) {

}

CompletionTrie::~CompletionTrie() {
	delete[] mem;
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
	u_int64_t firstNonSiblingPointer = firstFreeMemPointer;

	u_int64_t currentNode;
	PackedNode* n;
	do {
		n = (PackedNode*) (mem + nodePointer);
		currentNode = *((u_int64_t*) (n));

		/*
		 * Try if the N characters in the node match the next N characters of the prefix
		 */
		bool nodeFits = (currentNode >> 8 & characterMask[n->charactersSize_])
				== (*((u_int64_t*) (prefixChars + charPos))
						& characterMask[n->charactersSize_]);
		if (nodeFits) {
			resultLocus.push_back(n);
			firstNonSiblingPointer = 0;
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

/**
 *
 */
void CompletionTrie::addTerm(const std::string term, const u_int32_t score) {
	if (firstNodePointer == 0) {
		firstNodePointer = memSize / 2;

		const PackedNode *node = PackedNode::createNode(mem, firstNodePointer,
				term.length(), term.c_str(), true, 0xFFFFFFFF - score, 0,
				false);

		firstFreeMemPointer = firstNodePointer + node->getSize();
	} else {
		bool foundCompleteTerm = false;
		std::vector<PackedNode*> locus = findLocus(term, foundCompleteTerm);
		if (foundCompleteTerm) {
			// Term already exists!
			return;
		}

		PackedNode* parent = locus.back();
		u_int32_t deltaScore = 0xFFFFFFFF;
		int completeSuffixLength = 0;
		for (PackedNode* n : locus) {
			deltaScore -= n->getDeltaScore();
			completeSuffixLength += n->charactersSize_;
		}
		/*
		 * TODO: What if score is bigger than the score of the parent?
		 */
		deltaScore -= score;

		std::string suffix = term.substr(completeSuffixLength);
		std::cout << "suffix String: " << suffix << std::endl;

		/*
		 * Create a new Node on a separate memory to define the length and copy it to mem afterwards
		 *
		 * TODO: Calculation of the length of the Node might be faster instead of
		 * 		creating it on separate memory first
		 */
		const PackedNode* newNode = PackedNode::createNode(suffix.length(),
				suffix.c_str(), true, deltaScore - score, 0);

		const u_int64_t newNodePointer = makeRoomBehindNode(
				findLeftSibling(deltaScore - score, parent), parent,
				newNode->getSize());

		memcpy(mem + newNodePointer, newNode, newNode->getSize());

		delete (newNode);

	}
}

u_int64_t CompletionTrie::makeRoomBehindNode(PackedNode* node,
		PackedNode* parent, const uint width) {
	/*
	 * Calculate the additional shift due to increasing the firstChildOffset of the left siblings leading to
	 * an extension of some of those nodes as the firstChildOffset field has to be increased if all bits are
	 * already used
	 */
	u_int64_t siblingPointer = reinterpret_cast<u_int64_t>(parent);
	const u_int64_t firstFreeBytePtr = reinterpret_cast<u_int64_t>(node)
			+ node->getSize();
	PackedNode* sibling;

	uint bytesToShiftForFirstChildOffsetExtension = 0;
	while (siblingPointer < firstFreeBytePtr) {
		sibling = reinterpret_cast<PackedNode*>(mem + siblingPointer);
		siblingPointer += sibling->getSize();
		bytesToShiftForFirstChildOffsetExtension +=
				sibling->bytesToExtendOnFirstChildOffsetIncrementation(width);
	}

	memmove(
			node + node->getSize() + width
					+ bytesToShiftForFirstChildOffsetExtension,
			node + node->getSize(),
			firstFreeMemPointer - getPositionInMem(node) + node->getSize());

	/*
	 * update firstChildOffset values of all siblings to the left
	 */
	siblingPointer = reinterpret_cast<u_int64_t>(parent);
	while (siblingPointer < reinterpret_cast<u_int64_t>(node)) {
		sibling = reinterpret_cast<PackedNode*>(mem + siblingPointer);
		siblingPointer += sibling->getSize();
		/*
		 * TODO: to be implemented
		 */
	}
	return firstFreeBytePtr + bytesToShiftForFirstChildOffsetExtension;
}

/**
 * Returns the last child of parent with a higher score then defined by the given deltaScore
 */
PackedNode* CompletionTrie::findLeftSibling(const u_int32_t deltaScore,
		PackedNode* parent) {
	if (parent->firstChildOffsetSize_ == 0) {
		return parent;
	}

	u_int64_t siblingPointer = reinterpret_cast<u_int64_t>(parent)
			+ parent->getFirstChildOffset();

	const u_int64_t endOfSiblingsPointer = reinterpret_cast<PackedNode*>(mem
			+ siblingPointer)->getFirstChildOffset();

	while (siblingPointer < endOfSiblingsPointer) {

	}

//	while (siblingPointer < siblingEndPointer) {
//		sibling = reinterpret_cast<PackedNode*>(mem + siblingPointer);
//		siblingPointer += sibling->getSize();
//	}
}
