/*
 * CompletionTrie.h
 *
 *  Created on: Oct 23, 2013
 *      Author: Jonas Kunze
 */

#ifndef COMPLETIONTRIE_H_
#define COMPLETIONTRIE_H_

#include <sys/types.h>
#include <deque>
#include <memory>
#include <string>
#include <vector>

#include "PackedNode.h"

class SuggestionStore;

struct SuggestionList;

class PackedNode;

/*
 * This struct is used for the suggestion retrieval.
 * It stores a node with a score relative to the best fitting node of the
 * searched term and the prefix of the node to reconstruct the whole term
 */
struct NodeWithRelativeScoreStore {
	u_int32_t relativeScoreOfParent;
	PackedNode* node;
	std::string prefix;
	std::string getString() {
		return prefix + node->getString();
	}

	u_int32_t getRelativeScore() {
		return relativeScoreOfParent - node->getDeltaScore();
	}
};

class CompletionTrie {
public:
	CompletionTrie(char* mem, u_int64_t _memSize,
			std::shared_ptr<SuggestionStore> _suggestionStore);
	virtual ~CompletionTrie();

	/**
	 * Returns the Node that defines the longest string that has term as substring
	 *
	 * @param return_prefixPos The position of the last character in term not defined by the returned
	 *  Node
	 */
	PackedNode* findBestFitting(const std::string term, int& return_prefixPos,
			std::vector<NodeWithRelativeScoreStore>& return_fittingLeafNodes) const;

	std::shared_ptr<SuggestionList> getSuggestions(std::string prefix,
			const int k) const;

	void print() const;

	u_int64_t getMemoryConsumption() const {
		return memSize;
	}

private:
	PackedNode* root;

	/*
	 * The main memory used to store the trie
	 * TODO: Use a memory mapped file for persistency
	 */
	const char* mem;
	const u_int64_t memSize;

	std::shared_ptr<SuggestionStore> suggestionStore;

	/**
	 * Concatenates the characters of all nodes in the given locus
	 */
	static std::string generateStringFromLocus(std::vector<PackedNode*> locus) {
		std::string result;
		result.reserve(64);

		for (PackedNode* n : locus) {
			result.append(n->getCharacters(), n->charactersSize_);
		}
		return result;
	}

	/**
	 * Returns the first child of the given node. If the node does not have any children
	 * the node itself is returned
	 */
	PackedNode* getFirstChild(PackedNode* parent) const {
		return reinterpret_cast<PackedNode*>(reinterpret_cast<char*>(parent)
				+ parent->getFirstChildOffset());
	}

	/**
	 * Returns the next sibling of the given node. If the node does not have any sibling
	 * nullptr is returned
	 */
	PackedNode* getNextSibling(PackedNode* node) const {
		if (node->isLastSibling_) {
			return nullptr;
		}
		return reinterpret_cast<PackedNode*>(reinterpret_cast<char*>(node)
				+ node->getSize());
	}

	void printNode(PackedNode* parent, std::vector<PackedNode*> locus) const;
};

#endif /* COMPLETIONTRIE_H_ */
