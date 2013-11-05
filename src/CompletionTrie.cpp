/*
 * CompletionTrie.cpp
 *
 *  Created on: Oct 23, 2013
 *      Author: Jonas Kunze
 */

#include "CompletionTrie.h"

#include <iostream>
#include <map>
#include <utility>

#include "Suggestion.h"
#include "Utils.h"

CompletionTrie::CompletionTrie(char* _mem, u_int32_t _memSize) :
		root(reinterpret_cast<PackedNode*>(_mem)), mem(_mem), memSize(_memSize) {
}

CompletionTrie::~CompletionTrie() {
	delete[] mem;
}

std::shared_ptr<SimpleSuggestions> CompletionTrie::getSuggestions(
		std::string term, const int k) {
	std::shared_ptr<SimpleSuggestions> suggestions(new SimpleSuggestions(k));

	int termPrefixPos = 0;
	bool termFoundAsLeaf = false;
	std::vector<std::string> fittingLeafNodes;
	PackedNode* node = findBestFitting(term, termPrefixPos, fittingLeafNodes);

	term = term.substr(0, termPrefixPos);

	for (std::string s : fittingLeafNodes) {
		suggestions->addSuggestion(s);
	}

	if (node == root || node == NULL) {
		return suggestions;
	}

	std::map<u_int32_t, std::pair<PackedNode*, std::string>> nodes;
	nodes.insert(std::make_pair(0xFFFFFFFF, std::make_pair(node, term)));

	u_int32_t score;
	bool isFirstNode = true;
	while (!nodes.empty()) {
		score = nodes.rbegin()->first;
		auto pair = nodes.rbegin()->second;
		node = pair.first;
		std::string prefix = pair.second;
		nodes.erase(score);

		if (node->isLeafNode()) {
			suggestions->addSuggestion(
					prefix
							+ std::string(node->getCharacters(),
									node->charactersSize_));
			if (suggestions->isFull()) {
				return suggestions;
			}
		}

		if (node->firstChildOffsetSize_ != 0) {
			PackedNode* child = getFirstChild(node);
			nodes.insert(
					std::make_pair(score - child->getDeltaScore(),
							std::make_pair(child,
									prefix
											+ std::string(node->getCharacters(),
													node->charactersSize_))));
		}

		if (!isFirstNode) {
			PackedNode* sibling = getNextSibling(node);
			if (sibling != NULL) {
				nodes.insert(
						std::make_pair(score - sibling->getDeltaScore(),
								std::make_pair(sibling, prefix)));
			}
		} else {
			isFirstNode = false;
		}
	}

	return suggestions;
}

PackedNode* CompletionTrie::findBestFitting(const std::string term,
		int& return_prefixPos,
		std::vector<std::string>& return_fittingLeafNodeStrings) {

	uint charPos = 0;
	const char* prefixChars = term.c_str();
	u_int64_t node_ptr = reinterpret_cast<u_int64_t>(root)
			+ root->getFirstChildOffset();

	PackedNode* currentNode;
	PackedNode* lastFittingNode = NULL;
	bool nodeFits;
	do {
		currentNode = reinterpret_cast<PackedNode*>(node_ptr);

		/*
		 * Try if the N characters in the node match the next N characters of the prefix
		 */
//		nodeFits = (currentNode_value >> 8
//				& characterMask[currentNode->charactersSize_])
//				== (*((u_int64_t*) (prefixChars + charPos))
//						& characterMask[currentNode->charactersSize_]);
		nodeFits = Utils::findFirstNonMatchingCharacter(
				((char*) currentNode) + 1, prefixChars + charPos) > 0;

		if (nodeFits) {
			return_prefixPos = charPos;
			lastFittingNode = currentNode;

			if (currentNode->isLeafNode()) {
				/*
				 * The term is stored in the trie. One suggestion should be the term as such (return_fittingLeafNode)
				 * but we have to carry on searching for fitting nodes...
				 */
				node_ptr += currentNode->getSize();
				return_fittingLeafNodeStrings.push_back(
						term.substr(0, charPos + currentNode->charactersSize_));
				continue;
			}

			charPos += currentNode->charactersSize_;
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

	} while ((nodeFits && charPos <= term.length())
			|| ((charPos < term.length()
					|| return_fittingLeafNodeStrings.size() == 0)
					&& !currentNode->isLastSibling_));

	if (lastFittingNode != NULL && !return_fittingLeafNodeStrings.empty()
			&& term.substr(0, charPos) + lastFittingNode->getString()
					== return_fittingLeafNodeStrings[return_fittingLeafNodeStrings.size()
							- 1]) {
		return_fittingLeafNodeStrings.erase(
				return_fittingLeafNodeStrings.end());
	}

	return lastFittingNode;
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
