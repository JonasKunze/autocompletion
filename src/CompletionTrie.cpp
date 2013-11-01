/*
 * CompletionTrie.cpp
 *
 *  Created on: Oct 23, 2013
 *      Author: Jonas Kunze
 */

#include "CompletionTrie.h"

#include <deque>
#include <iostream>
#include <map>
#include <utility>

#include "Suggestion.h"

static u_int64_t characterMask[] = { 0, 0xFF, 0xFFFF, 0xFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFFFF, 0xFFFFFFFFFFFF, 0xFFFFFFFFFFFFFF };

CompletionTrie::CompletionTrie(char* _mem, u_int32_t _memSize) :
		root(reinterpret_cast<PackedNode*>(_mem)), mem(_mem), memSize(_memSize) {
}

CompletionTrie::~CompletionTrie() {
	delete[] mem;
}

std::shared_ptr<SimpleSuggestions> CompletionTrie::getSuggestions(
		std::string prefix, const int k) {
	std::shared_ptr<SimpleSuggestions> suggestions(new SimpleSuggestions(k));

	bool foundTerm;
	PackedNode* node = findBestFitting(prefix, foundTerm);

	std::map<u_int32_t, std::pair<PackedNode*, std::string>> nodes;
	nodes.insert(std::make_pair(0xFFFFFFFF, std::make_pair(node, prefix)));

	if (node == root) {
		return suggestions;
	}

	u_int32_t score;
	while (!nodes.empty()) {
		auto pair = nodes.rbegin()->second;
		node = pair.first;
		prefix = pair.second;
		score = nodes.rbegin()->first;
		nodes.erase(score);

		if (node->isLeafNode()) {
			suggestions->addSuggestion(
					prefix
							+ std::string(node->getCharacters(),
									node->charactersSize_));
			if (suggestions->isFull()) {
				return suggestions;
			}
		} else {
			PackedNode* child = getFirstChild(node);
			nodes.insert(
					std::make_pair(score - child->getDeltaScore(),
							std::make_pair(child,
									prefix
											+ std::string(node->getCharacters(),
													node->charactersSize_))));
		}
	}

	return suggestions;
}

PackedNode* CompletionTrie::findBestFitting(const std::string term,
		bool& return_foundTerm) {

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
			charPos += currentNode->charactersSize_;

			if (charPos == term.size()
					&& currentNode->firstChildOffsetSize_ == 0) {
				// we've reached the end of the term
				return_foundTerm = true;
				return currentNode;
			}

			if (currentNode->firstChildOffsetSize_ == 0) {
				// No more children
				// this node defines only a part of the term but we've reached the end
				// It's like searching for "autocompletion" but only "auto" exists
				return currentNode;
			}
			node_ptr += currentNode->getFirstChildOffset();
		} else {
			/*
			 * Move to the next sibling
			 */
			node_ptr += currentNode->getSize();
		}

	} while (nodeFits || (!nodeFits && !currentNode->isLastSibling_));

	return currentNode;
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
