/*
 * CompletionTrie.cpp
 *
 *  Created on: Oct 23, 2013
 *      Author: Jonas Kunze
 */

#include "CompletionTrie.h"

#include <cstring>

static u_int64_t characterMask[] = { 0, 0xFF, 0xFFFF, 0xFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFFFF, 0xFFFFFFFFFFFF, 0xFFFFFFFFFFFFFF };

CompletionTrie::CompletionTrie(char* _mem, u_int32_t _memSize) :
		root(reinterpret_cast<PackedNode*>(_mem)), mem(_mem), memSize(_memSize) {
}

CompletionTrie::~CompletionTrie() {
	delete[] mem;
}

/**
 * Returns the node defining as many characters of {term} as possible. return_foundTerm will be set to
 * true if the whole term is defined by the returned node
 *
 * @param term The string searched for
 * @param return_foundTerm true if the whole term is defined by the returned node
 *
 * @return The Node in the trie defining the maximum largest substring of term or the whole term
 */
std::deque<PackedNode*> CompletionTrie::findLocusWithSubstr(
		const std::string term, bool& return_foundTerm) {
	std::deque<PackedNode*> resultLocus;

	return_foundTerm = false;
	uint charPos = 0;
	const char* prefixChars = term.c_str();
	u_int64_t node_ptr = reinterpret_cast<u_int64_t>(root);

	u_int64_t currentNode_value;
	PackedNode* currentNode;
	int firstNonFittingByte;
	do {
		currentNode = reinterpret_cast<PackedNode*>(node_ptr);
		currentNode_value = *((u_int64_t*) (currentNode));

		/*
		 * XOR of the two strings. Should be 0 if they are the same. The first bit being 1 indicates
		 * the first characters that are unequal
		 */
		const long l1 = (currentNode_value >> 8)
				& characterMask[currentNode->charactersSize_];
		const long l2 = (*((u_int64_t*) (prefixChars + charPos)))
				& characterMask[currentNode->charactersSize_];

		firstNonFittingByte = ffsl(
				(l1 ^ l2) & characterMask[currentNode->charactersSize_]);
		firstNonFittingByte =
				(firstNonFittingByte == 0) ?
						0 : 1 + (firstNonFittingByte - 1) / 8;

		if (firstNonFittingByte == 0) {
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
	} while (firstNonFittingByte == 0
			|| (firstNonFittingByte != 0 && !currentNode->isLastSibling_));

	return resultLocus;
}

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

	} while (nodeFits || (!nodeFits && !currentNode->isLastSibling_));

	return resultLocus;
}

void CompletionTrie::print() {
	u_int64_t node_ptr = reinterpret_cast<u_int64_t>(root)
			+ root->getFirstChildOffset();
	do {
		PackedNode* node = reinterpret_cast<PackedNode*>(node_ptr);
		PackedNode* firstChild = reinterpret_cast<PackedNode*>(node_ptr
				+ node->getFirstChildOffset());
		std::cout << node_ptr << "\t\""
				<< std::string(node->getCharacters(), node->charactersSize_)
				<< "\"\t" << (int) node->getSize() << "\t"
				<< (int) (u_int8_t) (node->getFirstChildOffset()) << "\t\""
				<< std::string(firstChild->getCharacters(),
						firstChild->charactersSize_) << "\"" << "\t"
				<< node->isLastSibling_ << std::endl;
		node_ptr += node->getSize();
	} while (node_ptr < reinterpret_cast<u_int64_t>(mem) + memSize);

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

		if (child->isLastSibling_) {
			break;
		} else {
			child = reinterpret_cast<PackedNode*>(reinterpret_cast<char*>(child)
					+ child->getSize());
		}
	}
}
