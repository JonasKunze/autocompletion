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
#include <string>
#include <vector>

#include "PackedNode.h"

static u_int64_t characterMask[] = { 0, 0xFF, 0xFFFF, 0xFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFFFF, 0xFFFFFFFFFFFF, 0xFFFFFFFFFFFFFF };
CompletionTrie::CompletionTrie() :
		root(NULL), mem(new char[initialMemSize]), memSize(initialMemSize), firstFreeMem_ptr(
				reinterpret_cast<u_int64_t>(mem)) {
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
std::deque<PackedNode*> CompletionTrie::findLocus(const std::string term,
		bool& return_foundTerm) {
	std::deque<PackedNode*> resultLocus;

	return_foundTerm = false;
	uint charPos = 0;
	const char* prefixChars = term.c_str();
	u_int64_t node_ptr = reinterpret_cast<u_int64_t>(root);
	u_int64_t firstNonSibling_ptr = firstFreeMem_ptr;

	u_int64_t currentNode;
	PackedNode* n;
	do {
		n = (PackedNode*) node_ptr;
		currentNode = *((u_int64_t*) (n));

		/*
		 * Try if the N characters in the node match the next N characters of the prefix
		 */
		bool nodeFits = (currentNode >> 8 & characterMask[n->charactersSize_])
				== (*((u_int64_t*) (prefixChars + charPos))
						& characterMask[n->charactersSize_]);
		if (nodeFits) {
			resultLocus.push_back(n);
			firstNonSibling_ptr = 0;
			charPos += n->charactersSize_;

			/*
			 * Is
			 */
			if (n->firstChildOffsetSize_ == 0) {
				// No more children
				if (charPos == term.size() && n->firstChildOffsetSize_ == 0) {
					// we've reached the end of the term
					return_foundTerm = true;
					return resultLocus;
				} else {
					// this node defines only a part of the term but we've reached the end
					// It's like searching for "autocompletion" but only "auto" exists
					return resultLocus;
				}
			}
			node_ptr += n->getFirstChildOffset();
		} else {
			if (firstNonSibling_ptr == 0) {
				/*
				 * nodeFits is false for the first time at the current depth
				 * Save the _ptr of the first child as iterating through all siblings must be stopped there
				 */
				firstNonSibling_ptr = node_ptr + n->getFirstChildOffset();
			}
			/*
			 * Move to the next sibling
			 */
			node_ptr += n->getSize();
		}

	} while (firstNonSibling_ptr == 0 || node_ptr < firstNonSibling_ptr);

	return resultLocus;
}

/**
 *
 */
void CompletionTrie::addTerm(const std::string term, const u_int32_t score) {
	if (root == NULL) {
		root = PackedNode::createRootNode(mem);

		const PackedNode* node = PackedNode::createNode(
				reinterpret_cast<char*>(root) + root->getFirstChildOffset(),
				term.length(), term.c_str(), true, 0xFFFFFFFF - score, 0);

		firstFreeMem_ptr = reinterpret_cast<u_int64_t>(node) + node->getSize();
	} else {
		bool foundCompleteTerm = false;
		std::deque<PackedNode*> locus = findLocus(term, foundCompleteTerm);
		if (foundCompleteTerm) {
			// Term already exists!
			return;
		}

		/*
		 * TODO: What if score is bigger than the score of the parent?
		 */
		u_int32_t deltaScore = 0xFFFFFFFF - score;
		std::string suffix;
		if (locus.size() == 1) {
			/*
			 * No suffix node found (only root is in locus)
			 */
			suffix = term;
		} else {
			int completeSuffixLength = 0;
			for (PackedNode* n : locus) {
				deltaScore -= n->getDeltaScore();
				completeSuffixLength += n->charactersSize_;
			}
			suffix = term.substr(completeSuffixLength);
		}

		/*
		 * Create a new Node on a separate memory to define the length and copy it to mem afterwards
		 *
		 * TODO: Calculation of the length of the Node is faster instead of creating it on separate memory first
		 *
		 * TODO: check if suffix.length() > 7
		 */
		PackedNode* newNode = PackedNode::createNode(suffix.length(),
				suffix.c_str(), false, deltaScore - score, 0);

		PackedNode* parent = locus.back();

		u_int64_t newNode_ptr;
		bool nodeIsLastSibling = false;
		if (parent->firstChildOffsetSize_ == 0) {
			newNode_ptr = makeRoomBehindNode(parent, locus,
					newNode->getSize() + 1, nodeIsLastSibling) + 1;
			parent->setFirstChildOffset(parent->getSize() + 1);
		} else {
			newNode_ptr = makeRoomBehindNode(
					findLeftSibling(deltaScore - score, parent), locus,
					newNode->getSize(), nodeIsLastSibling);
		}
		newNode->isLastSibling = nodeIsLastSibling;
		memcpy(reinterpret_cast<char*>(newNode_ptr), newNode,
				newNode->getSize());

		delete (newNode);
	}
}

u_int64_t CompletionTrie::makeRoomBehindNode(PackedNode* node,
		std::deque<PackedNode*> parentLocus, const uint width,
		bool& nodeIsLastSibling) {

	/*
	 * The first byte of the new space if the left siblings would not change their length
	 */
	const u_int64_t firstRoomByte = reinterpret_cast<u_int64_t>(node)
			+ node->getSize();

	/*
	 * Calculate the additional shift due to increasing the firstChildOffset of the left siblings leading to
	 * an extension of some of those nodes as the firstChildOffset field has to be increased if all bits are
	 * already used
	 */
	PackedNode* parent = parentLocus.back();
	uint bytesToShiftForFirstChildOffsetExtension = 0;
	if (parent->firstChildOffsetSize_ != 0) {
		u_int64_t sibling_ptr = reinterpret_cast<u_int64_t>(parent)
				+ parent->getFirstChildOffset();

		PackedNode* sibling;

		while (sibling_ptr < firstRoomByte) {
			sibling = reinterpret_cast<PackedNode*>(sibling_ptr);
			sibling_ptr += sibling->getSize();
			if (sibling->firstChildOffsetSize_ != 0) {
				bytesToShiftForFirstChildOffsetExtension +=
						sibling->bytesToExtendOnFirstChildOffsetChange(
								sibling->getFirstChildOffset() + width);
			}
			/*
			 * The new node will be the last sibling if the last of the left siblings was it
			 */
			if (sibling->isLastSibling) {
				sibling->isLastSibling = false;
				nodeIsLastSibling = true;
			}
		}
	} else {
		nodeIsLastSibling = true;
	}

	memmove(
			node + node->getSize() + width
					+ bytesToShiftForFirstChildOffsetExtension,
			node + node->getSize(),
			firstFreeMem_ptr - 1 - reinterpret_cast<u_int64_t>(node)
					+ node->getSize());
	/*
	 * update firstChildOffset values of all siblings to the left
	 */
	u_int64_t sibling_ptr = reinterpret_cast<u_int64_t>(parent)
			+ parent->getFirstChildOffset();
	uint shift = bytesToShiftForFirstChildOffsetExtension + width;

	while (sibling_ptr < firstRoomByte) {
		PackedNode* sibling = reinterpret_cast<PackedNode*>(sibling_ptr);
		if (sibling->firstChildOffsetSize_ != 0) {
			int8_t sizeChange = sibling->bytesToExtendOnFirstChildOffsetChange(
					sibling->getFirstChildOffset() + width);
			moveRightSiblings(sibling, sizeChange);

			sibling->setFirstChildOffset(
					sibling->getFirstChildOffset() + shift);
			shift -= sizeChange;
		}
		sibling_ptr += sibling->getSize();
	}
	firstFreeMem_ptr += width + bytesToShiftForFirstChildOffsetExtension;
	return firstRoomByte + bytesToShiftForFirstChildOffsetExtension;
}

void CompletionTrie::moveRightSiblings(PackedNode* leftSibling,
		const uint width) {
	PackedNode* currentSibling = leftSibling;

	while (!currentSibling->isLastSibling) {
		currentSibling =
				reinterpret_cast<PackedNode*>(reinterpret_cast<char*>(currentSibling)
						+ currentSibling->getSize());
	}
	memmove(
			reinterpret_cast<char*>(leftSibling) + leftSibling->getSize()
					+ width,
			reinterpret_cast<char*>(leftSibling) + leftSibling->getSize(),
			reinterpret_cast<char*>(currentSibling) + currentSibling->getSize()
					- reinterpret_cast<char*>(leftSibling)
					+ leftSibling->getSize());
}

/**
 * Returns the last child of parent with a higher score then defined by the given deltaScore
 * parent->firstChildOffsetSize_ may not be 0!
 */
PackedNode* CompletionTrie::findLeftSibling(const u_int32_t deltaScore,
		PackedNode* parent) {
	u_int64_t sibling_ptr = reinterpret_cast<u_int64_t>(parent)
			+ parent->getFirstChildOffset();

	do {
		PackedNode* sibling = reinterpret_cast<PackedNode*>(sibling_ptr);
		if (sibling->isLastSibling || sibling->getDeltaScore() <= deltaScore) {
			return sibling;
		}
		sibling_ptr += sibling->getSize();
	} while (true);
	return NULL;
}

void CompletionTrie::print() {
	std::deque<PackedNode*> locus;
	std::cout << "graph completionTrie {" << std::endl;

	locus.push_back(root);
	printNode(root, locus);

	std::cout << "}" << std::endl;
}

/**
 * Recursively prints a node and all its children in the dot format "parent -- child"
 */
void CompletionTrie::printNode(PackedNode* parent,
		std::deque<PackedNode*> locus) {
	PackedNode* child = getFirstChild(parent);
	if (child == parent) {
		return;
	}
	while (true) {
		locus.push_back(child);
		if (child->firstChildOffsetSize_ != 0) {
			printNode(child, locus);
		}
		locus.pop_back();

		if (parent->charactersSize_ == 0) {
			/*
			 * the root element
			 */
			std::cout << "ROOT";
		} else {
			std::cout
					<< std::string(parent->getCharacters(),
							parent->charactersSize_);
		}
		std::cout << " -- "
				<< std::string(child->getCharacters(), child->charactersSize_)
				<< std::endl;

		if (child->isLastSibling) {
			break;
		} else {
			child = reinterpret_cast<PackedNode*>(reinterpret_cast<char*>(child)
					+ child->getSize());
		}
	}
}
