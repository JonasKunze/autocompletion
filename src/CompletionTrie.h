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
#include <map>
#include <iostream>
#include <string>

#include "PackedNode.h"

struct SimpleSuggestions;

class PackedNode;

class CompletionTrie {
public:
	CompletionTrie();
	virtual ~CompletionTrie();

	void addTerm(const std::string term, const u_int32_t score);
	std::deque<PackedNode*> findLocus(const std::string term,
			bool& return_foundTerm);
	SimpleSuggestions* getSuggestions(const std::string prefix, const int k);
	char* mem;

	void print();
private:
	static const long initialMemSize = 80; //1 << 20;

	PackedNode* root;

	/*
	 * The main memory used to store the trie
	 * TODO: Use a memory mapped file instead
	 */

	long memSize;

	u_int64_t firstFreeMemPointer;

	/**
	 * Shifts all Nodes right of node to the right by width bytes and updates the
	 * firstChildOffset of all left siblings
	 *
	 * @return A pointer to the first free byte (with width following free bytes)
	 */
	u_int64_t makeRoomBehindNode(PackedNode* node,
			std::deque<PackedNode*> parentLocus, const uint width);

	PackedNode* findLeftSibling(const u_int32_t deltaScore, PackedNode* parent);

	/**
	 * Returns the first child of the given node. If the node does not have any children
	 * the node itself is returned
	 */
	PackedNode* getFirstChild(PackedNode* parent) {
		return reinterpret_cast<PackedNode*>(reinterpret_cast<char*>(parent)
				+ parent->getFirstChildOffset());
	}

	/**
	 * Returns a pointer to the first byte behind the last child of the given node. If the node does not have
	 * any children 0 is returned
	 */
	u_int64_t getFirstByteBehindChildren(PackedNode* parent,
			std::string nodeTerm) {
		if (parent->firstChildOffsetSize_ == 0) {
			return 0;
		}

		PackedNode* child = getFirstChild(parent);
		while (true) {
			if (child->firstChildOffsetSize_ != 0) {
				return reinterpret_cast<u_int64_t>(reinterpret_cast<char*>(child)
						+ child->getFirstChildOffset());
			}
			if (child->isLastSibling) {
				break;
			} else {
				child =
						reinterpret_cast<PackedNode*>(reinterpret_cast<char*>(child)
								+ child->getSize());
			}
		}
		/*
		 * No Child with children found->First free byte is behind the last child
		 */
		return reinterpret_cast<u_int64_t>(reinterpret_cast<char*>(child)
				+ child->getSize());
	}

	/**
	 * Concatenates the characters of all nodes in the given locus
	 */
	static std::string generateStringFromLocus(std::deque<PackedNode*> locus) {
		std::string result;
		result.reserve(64);

		for (PackedNode* n : locus) {
			result.append(n->getCharacters(), n->charactersSize_);
		}
		return result;
	}

	void printNode(PackedNode* parent, std::deque<PackedNode*> locus);
};

#endif /* COMPLETIONTRIE_H_ */
