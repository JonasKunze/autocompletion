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

struct SimpleSuggestions;

class PackedNode;

class CompletionTrie {
public:
	CompletionTrie(char* mem, u_int32_t _memSize);
	virtual ~CompletionTrie();

	void addTerm(const std::string term, const u_int32_t score);
	std::deque<PackedNode*> findLocusWithSubstr(const std::string term,
			bool& return_foundTerm);

	PackedNode* findBestFitting(const std::string term, bool& return_foundTerm);

	std::shared_ptr<SimpleSuggestions> getSuggestions(std::string prefix,
			const int k);

	void print();

private:
	PackedNode* root;

	char* mem;

	/*
	 * The main memory used to store the trie
	 * TODO: Use a memory mapped file for persistency
	 */
	u_int32_t memSize;

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
	PackedNode* getFirstChild(PackedNode* parent) {
		return reinterpret_cast<PackedNode*>(reinterpret_cast<char*>(parent)
				+ parent->getFirstChildOffset());
	}

	void printNode(PackedNode* parent, std::deque<PackedNode*> locus);
};

#endif /* COMPLETIONTRIE_H_ */
