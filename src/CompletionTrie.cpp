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

	u_int64_t currentNode_value;
	PackedNode* currentNode;
	bool nodeFits;
	do {
		currentNode = reinterpret_cast<PackedNode*>(node_ptr);
		currentNode_value = *((u_int64_t*) (currentNode));

		/*
		 * Try if the N characters in the node match the next N characters of the prefix
		 */
		nodeFits = (currentNode_value >> 8
				& characterMask[currentNode->charactersSize_])
				== (*((u_int64_t*) (prefixChars + charPos))
						& characterMask[currentNode->charactersSize_]);
		if (nodeFits) {
			resultLocus.push_back(currentNode);
			charPos += currentNode->charactersSize_;

			if (charPos == term.size()
					&& currentNode->firstChildOffsetSize_ == 0) {
				// we've reached the end of the term
				return_foundTerm = true;
				return resultLocus;
			}

			if (currentNode->firstChildOffsetSize_ == 0) {
				// No more children
				// this node defines only a part of the term but we've reached the end
				// It's like searching for "autocompletion" but only "auto" exists
				return resultLocus;
			}
			node_ptr += currentNode->getFirstChildOffset();
		} else {
			/*
			 * Move to the next sibling
			 */
			node_ptr += currentNode->getSize();
		}

	} while (nodeFits || (!nodeFits && !currentNode->isLastSibling));

	return resultLocus;
}

/**
 *
 */
void CompletionTrie::addTerm(const std::string term, const u_int32_t score) {
	if (root == NULL) {
		root = PackedNode::createRootNode(mem);

		PackedNode* node = PackedNode::createNode(
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
				suffix.c_str(), false, deltaScore, 0);

		PackedNode* parent = locus.back();

		u_int64_t newNode_ptr;
		bool nodeIsLastSibling = false;
		if (parent->firstChildOffsetSize_ == 0) {
			PackedNode* futureNodeToTheLeft = parent;
			if (locus.size() != 1) {
				locus.pop_back();
				PackedNode* grandparent = locus.back();
				locus.push_back(parent);

				PackedNode* parentsLeftSiblingWithChild =
						findFirstLeftSiblingWithChild(parent, grandparent);
				if (parentsLeftSiblingWithChild != NULL) {
					futureNodeToTheLeft = getLastChild(
							parentsLeftSiblingWithChild);
				}
			}

			/*
			 * +1 as the parent node will by increased by 1 byte as it now hast to store
			 * the first child offset
			 */
			newNode_ptr = makeRoomBehindNode(futureNodeToTheLeft, locus,
					newNode->getSize() + 1, nodeIsLastSibling) + 1;

			/*
			 * Make room for the first child offset byte of parent
			 */
			const u_int64_t endOfParent_ptr =
					reinterpret_cast<u_int64_t>(parent) + parent->getSize();
			memmove(reinterpret_cast<char*>(endOfParent_ptr + 1),
					reinterpret_cast<char*>(endOfParent_ptr),
					newNode_ptr - endOfParent_ptr + parent->getSize());

			parent->setFirstChildOffset(
					reinterpret_cast<u_int64_t>(futureNodeToTheLeft)
							- reinterpret_cast<u_int64_t>(parent)
							+ futureNodeToTheLeft->getSize() + 1);
		} else {
			PackedNode* leftSibling = findLeftSiblingWithHigherScore(
					deltaScore - score, parent);
			newNode_ptr = makeRoomBehindNode(leftSibling, locus,
					newNode->getSize(), nodeIsLastSibling);
			leftSibling->isLastSibling = false;

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
	 * Calculate the additional shift due to increasing the firstChildOffset of the left siblings and parents
	 * right siblings leading to an extension of some of those nodes as the firstChildOffset field has to be
	 * increased if all bits are already used
	 */
	PackedNode* parent = parentLocus.back();
	uint bytesToShiftForFirstChildOffsetExtension = 0;
	if (parent->firstChildOffsetSize_ != 0) {
		u_int64_t parentsLastSibling_ptr = reinterpret_cast<u_int64_t>(parent)
				+ parent->getFirstChildOffset();

		// Start with the first right sibling of parent
		u_int64_t sibling_ptr = reinterpret_cast<u_int64_t>(parent)
				+ parent->getSize();

		PackedNode* sibling;
		while (sibling_ptr < firstRoomByte) {
			sibling = reinterpret_cast<PackedNode*>(sibling_ptr);
			if (sibling->firstChildOffsetSize_ != 0) {
				bytesToShiftForFirstChildOffsetExtension +=
						sibling->bytesToExtendOnFirstChildOffsetChange(
								sibling->getFirstChildOffset() + width);
			}
			/*
			 * The new node will be the last sibling if the last of the left siblings was it
			 */
			if (sibling->isLastSibling
					&& sibling_ptr > parentsLastSibling_ptr) {
				sibling->isLastSibling = false;
				nodeIsLastSibling = true;
			}
			sibling_ptr += sibling->getSize();
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

	memset(
			reinterpret_cast<char*>(firstRoomByte
					+ bytesToShiftForFirstChildOffsetExtension), 0xAA,
			width + bytesToShiftForFirstChildOffsetExtension);

	/*
	 * update firstChildOffset values of all nodes between node and parent
	 */
	u_int64_t sibling_ptr = reinterpret_cast<u_int64_t>(parent)
			+ parent->getSize();
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
		const int width) {
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
PackedNode* CompletionTrie::findLeftSiblingWithHigherScore(
		const u_int32_t deltaScore, PackedNode* parent) {
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

/**
 * Returns the last child of parent left of node with a child
 * parent->firstChildOffsetSize_ may not be 0!
 */
PackedNode* CompletionTrie::findFirstLeftSiblingWithChild(PackedNode* node,
		PackedNode* parent) {
	u_int64_t sibling_ptr = reinterpret_cast<u_int64_t>(parent)
			+ parent->getFirstChildOffset();
	PackedNode* lastSiblingWithChild = NULL;
	PackedNode* sibling;
	do {
		sibling = reinterpret_cast<PackedNode*>(sibling_ptr);

		if (sibling->firstChildOffsetSize_ != 0) {
			lastSiblingWithChild = sibling;
		}

		sibling_ptr += sibling->getSize();
	} while (!sibling->isLastSibling
			&& sibling_ptr < reinterpret_cast<u_int64_t>(node));
	return lastSiblingWithChild;
}

/**
 * Returns the last child of parent
 * parent->firstChildOffsetSize_ may not be 0!
 */
PackedNode* CompletionTrie::getLastChild(PackedNode* parent) {
	u_int64_t sibling_ptr = reinterpret_cast<u_int64_t>(parent)
			+ parent->getFirstChildOffset();
	PackedNode* sibling;
	do {
		sibling = reinterpret_cast<PackedNode*>(sibling_ptr);
		sibling_ptr += sibling->getSize();
	} while (!sibling->isLastSibling);
	return sibling;
}

void CompletionTrie::print() {
	u_int64_t node_ptr = reinterpret_cast<u_int64_t>(root)
			+ root->getFirstChildOffset();
	do {
		PackedNode* node = reinterpret_cast<PackedNode*>(node_ptr);
		std::cout << node_ptr << "\t\""
				<< std::string(node->getCharacters(), node->charactersSize_)
				<< "\"\t" << node->getSize() << "\t"
				<< node->getFirstChildOffset() - node->getSize() << std::endl;
		node_ptr += node->getSize();
	} while (node_ptr < firstFreeMem_ptr);

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
