/*
 * CompletionTrie.cpp
 *
 *  Created on: Oct 23, 2013
 *      Author: Jonas Kunze
 */

#include "CompletionTrie.h"

#include <algorithm>
#include <iostream>
#include <iterator>

#include "Suggestion.h"
#include "Utils.h"

struct NodeWithParentScoreStore {
	u_int32_t parentScore;
	PackedNode* node;
	std::string prefix;
};

struct NodeWithScoreStoreComparator {
	bool operator()(const NodeWithParentScoreStore left,
			const NodeWithParentScoreStore right) {
		return left.parentScore - left.node->getDeltaScore()
				> right.parentScore - right.node->getDeltaScore();
	}
};

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
	std::vector<std::string> fittingLeafNodes;
	PackedNode* node = findBestFitting(term, termPrefixPos, fittingLeafNodes);
	std::vector<PackedNode*> v;

	term = term.substr(0, termPrefixPos);

	for (std::string s : fittingLeafNodes) {
		suggestions->addSuggestion(s, 0);
	}

	if (node == root || node == nullptr) {
		return suggestions;
	}

	std::vector<NodeWithParentScoreStore> nodesByParentScore;
	nodesByParentScore.push_back( { 0xFFFFFFFF, node, term });

	bool isFirstNode = true;
	while (!nodesByParentScore.empty()) {
		std::sort(nodesByParentScore.begin(), nodesByParentScore.end(),
				NodeWithScoreStoreComparator());
		NodeWithParentScoreStore nodeWithParentScore =
				*nodesByParentScore.rbegin();
		nodesByParentScore.pop_back();

		if (nodeWithParentScore.node->isLeafNode()) {
			suggestions->addSuggestion(
					nodeWithParentScore.prefix
							+ nodeWithParentScore.node->getString(),
					nodeWithParentScore.parentScore
							- nodeWithParentScore.node->getDeltaScore());
			if (suggestions->isFull()) {
				return suggestions;
			}
		}

		/*
		 * Push first child to priority queue
		 */
		if (nodeWithParentScore.node->firstChildOffsetSize_ != 0) {
			PackedNode* child = getFirstChild(nodeWithParentScore.node);
			nodesByParentScore.push_back(
					{ nodeWithParentScore.parentScore
							- nodeWithParentScore.node->getDeltaScore(), child,
							nodeWithParentScore.prefix
									+ nodeWithParentScore.node->getString() });
		}

		/*
		 * Push next sibling to priority queue
		 */
		if (!isFirstNode) {
			PackedNode* sibling = getNextSibling(nodeWithParentScore.node);
			if (sibling != nullptr) {
				nodesByParentScore.push_back( { nodeWithParentScore.parentScore,
						sibling, nodeWithParentScore.prefix });
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
	PackedNode* lastFittingNode = nullptr;
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

	if (lastFittingNode != nullptr && !return_fittingLeafNodeStrings.empty()
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
	int layer = 0;
	do {
		PackedNode* node = reinterpret_cast<PackedNode*>(node_ptr);
		PackedNode* firstChild = reinterpret_cast<PackedNode*>(node_ptr
				+ node->getFirstChildOffset());
		std::cout << node->getString() << "\t"

		<< (int) node->getSize() << "\t"
				<< (int) (u_int8_t) (node->getFirstChildOffset()) << "\t"
				<< "\t" << firstChild->getString() << "\t" << layer
				<< std::endl;

		if (node->isLastSibling_) {
			layer++;
		}

		node_ptr += node->getSize();
	} while (node_ptr < reinterpret_cast<u_int64_t>(mem) + memSize);

	std::vector<PackedNode*> locus;
	std::cout << "graph completionTrie {" << std::endl;

	locus.push_back(root);
	printNode(root, locus);

	std::cout << "}" << std::endl;
}

/**
 * Recursively prints a node and all its children in the dot format "parent -- child"
 */
void CompletionTrie::printNode(PackedNode* parent,
		std::vector<PackedNode*> locus) {
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
		std::cout << " -- " << child->getString() << "\t"
				<< child->getDeltaScore() << std::endl;

		if (child->isLastSibling_) {
			break;
		} else {
			child = reinterpret_cast<PackedNode*>(reinterpret_cast<char*>(child)
					+ child->getSize());
		}
	}
}
