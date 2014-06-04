/*
 * CompletionTrie.cpp
 *
 *  Created on: Oct 23, 2013
 *      Author: Jonas Kunze
 */

#include "CompletionTrie.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <iterator>

#include "../options/Options.h"
#include "SuggestionList.h"
#include "SuggestionStore.h"
#include "../utils/Utils.h"

struct NodeWithScoreStoreComparator {
	bool operator()(const NodeWithRelativeScoreStore left,
			const NodeWithRelativeScoreStore right) {
		return left.relativeScoreOfParent - left.node->getDeltaScore()
				< right.relativeScoreOfParent - right.node->getDeltaScore();
	}
};

CompletionTrie::CompletionTrie(char* _mem, u_int64_t _memSize,
		std::shared_ptr<SuggestionStore> _suggestionStore) :
		root(reinterpret_cast<PackedNode*>(_mem)), mem(_mem), memSize(_memSize), suggestionStore(
				_suggestionStore) {
}

CompletionTrie::~CompletionTrie() {
	delete[] mem;
}

std::shared_ptr<SuggestionList> CompletionTrie::getSuggestions(std::string term,
		const int k) const {
	std::transform(term.begin(), term.end(), term.begin(), ::tolower);
	auto suggestions = suggestionStore->getSuggestionList(k);

	int termPrefixPos = 0;

	std::vector<NodeWithRelativeScoreStore> fittingLeafNodes;
	PackedNode* node = findBestFitting(term, termPrefixPos, fittingLeafNodes);
	std::vector<PackedNode*> v;

	/*
	 * TODO if term is wihtin fittingLeafNodes this must be shown as first element!
	 */

	term = term.substr(0, termPrefixPos);

	if (node == root || node == nullptr) {
		for (NodeWithRelativeScoreStore n : fittingLeafNodes) {
			suggestions->addSuggestion(n);
		}
		return suggestions;
	}

	std::vector<NodeWithRelativeScoreStore> nodesByParentScore;
	nodesByParentScore.push_back( { 0xFFFFFFFF, node, term });

	bool isFirstNode = true;
	while (!nodesByParentScore.empty()) {
		std::sort(nodesByParentScore.begin(), nodesByParentScore.end(),
				NodeWithScoreStoreComparator());

		NodeWithRelativeScoreStore nodeWithParentScore =
				*nodesByParentScore.rbegin();
		nodesByParentScore.pop_back();

//		std::cout << nodeWithParentScore.getString() << std::endl;

		if (nodeWithParentScore.node->isLeafNode()) {
			suggestions->addSuggestion(nodeWithParentScore);
			if (suggestions->isFull()) {
				return suggestions;
			}
		}

		/*
		 * Push first child to priority queue
		 */
		if (nodeWithParentScore.node->firstChildOffsetSize_ != 0) {
			PackedNode* child = getFirstChild(nodeWithParentScore.node);
			nodesByParentScore.push_back( {
					nodeWithParentScore.getRelativeScore(), child,
					nodeWithParentScore.getString() });
		}

		/*
		 * Push next sibling to priority queue
		 */
		if (!isFirstNode) {
			PackedNode* sibling = getNextSibling(nodeWithParentScore.node);
			if (sibling != nullptr) {
				nodesByParentScore.push_back(
						{ nodeWithParentScore.relativeScoreOfParent, sibling,
								nodeWithParentScore.prefix });
			}
		} else {
			isFirstNode = false;
		}
	}

	/*
	 * TODO: where should be put the nodes defining substrings of the requested term?
	 */
	return suggestions;
}

PackedNode* CompletionTrie::findBestFitting(const std::string term,
		int& return_prefixPos,
		std::vector<NodeWithRelativeScoreStore>& return_fittingLeafNodes) const {

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
				((char*) &currentNode->characters_deltaScore_firstChildOffset_),
				prefixChars + charPos) != 0;

		if (nodeFits) {
			return_prefixPos = charPos;
			lastFittingNode = currentNode;

			if (currentNode->isLeafNode()) {
				/*
				 * The term is stored in the trie. One suggestion should be the term as such (return_fittingLeafNode)
				 * but we might have to carry on searching for fitting nodes...
				 */
				return_fittingLeafNodes.push_back(
						{ currentNode->getDeltaScore(), currentNode,
								term.substr(0, charPos) });

				if (currentNode->isLastSibling_) {
					break;
				}
				node_ptr += currentNode->getSize();
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
			if (currentNode->isLastSibling_) {
				break;
			}
			node_ptr += currentNode->getSize();
		}

	} while (charPos < term.length());

	if (lastFittingNode != nullptr
			&& lastFittingNode->firstChildOffsetSize_ == 0) {
		/*
		 * It's already stored in return_fittingLeafNodes but we must return null als we
		 * didnt find any fitting locus
		 */
		lastFittingNode = nullptr;
	} else {
		/*
		 * Remove the lastFittingNode from the return_fittingLeafNode as it will be returned directly
		 */
		if (lastFittingNode != nullptr && !return_fittingLeafNodes.empty()
				&& term.substr(0, charPos) + lastFittingNode->getString()
						== (return_fittingLeafNodes.back()).getString()) {
			return_fittingLeafNodes.erase(return_fittingLeafNodes.end());
		}
	}

	return lastFittingNode;
}

void CompletionTrie::print() const {
	if (!Options::VERBOSE) {
		return;
	}
//	std::cout << "============ CompletionTrie ============" << std::endl;
//	u_int64_t node_ptr = reinterpret_cast<u_int64_t>(root)
//			+ root->getFirstChildOffset();
//	int layer = 0;
//
//	std::cout << "Name\tsize\tdeltaScore\t\tfirstChild\tLayer"<<std::endl;
//	do {
//		PackedNode* node = reinterpret_cast<PackedNode*>(node_ptr);
//		PackedNode* firstChild = reinterpret_cast<PackedNode*>(node_ptr
//				+ node->getFirstChildOffset());
//
//		std::cout << node->getString() << "\t" << (int) node->getSize() << "\t"
//				<< (int) (u_int8_t) (node->getDeltaScore()) << "\t" << "\t"
//				<< firstChild->getString() << "\t" << layer << std::endl;
//
//		if (node->isLastSibling_) {
//			layer++;
//		}
//
//		node_ptr += node->getSize();
//	} while (node_ptr < reinterpret_cast<u_int64_t>(mem) + memSize - 8/*safety margin*/);

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
		std::vector<PackedNode*> locus) const {
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
		std::cout << " -- " << child->getString() << "[ label = \""<< child->getDeltaScore() <<"\" ];" << std::endl;

		if (child->isLastSibling_) {
			break;
		} else {
			child = reinterpret_cast<PackedNode*>(reinterpret_cast<char*>(child)
					+ child->getSize());
		}
	}
}
