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
	std::deque<PackedNode*> findLocusWithSubstr(const std::string term,
			bool& return_foundTerm);

	std::deque<PackedNode*> findLocus(const std::string term,
			bool& return_foundTerm);

	SimpleSuggestions* getSuggestions(const std::string prefix, const int k);

	void print();
private:
	static const long initialMemSize = 80; //1 << 20;

	PackedNode* root;

	char* mem;

	/*
	 * The main memory used to store the trie
	 * TODO: Use a memory mapped file for persistency
	 */
	long memSize;

	u_int64_t firstFreeMem_ptr;

	/**
	 * Shifts all Nodes right of node to the right by width bytes and updates the
	 * firstChildOffset of all left siblings
	 *
	 * @param roomForNewNode Set to true if the new room will be used for a new node. In
	 * this case the left sibling's isLastSibling flag will be set to false
	 *
	 * @return A pointer to the first free byte (with width following free bytes)
	 */
	u_int64_t makeRoomBehindNode(PackedNode* node,
			std::deque<PackedNode*> parentLocus, const uint width,
			bool& nodeIsLastSibling, const bool roomForNewNode);

	/**
	 * Shifts all nodes right of leftNode to the right by width bytes WITHOUT
	 * updating any child offset
	 *
	 * After that the last moved byte will be width left of stopPointer
	 */
	void moveRightNodes(PackedNode* leftNode, const u_int64_t stopPointer,
			const int width);

	PackedNode* findLeftSiblingWithHigherScore(const u_int32_t deltaScore,
			PackedNode* parent);

	PackedNode* findFirstLeftSiblingWithChild(PackedNode* node,
			PackedNode* parent);

	PackedNode* getLastChild(PackedNode* parent);
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
