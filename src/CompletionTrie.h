/*
 * CompletionTrie.h
 *
 *  Created on: Oct 23, 2013
 *      Author: Jonas Kunze
 */

#ifndef COMPLETIONTRIE_H_
#define COMPLETIONTRIE_H_

#include <sys/types.h>
#include <string>
#include <vector>

#include "PackedNode.h"

struct SimpleSuggestions;

class PackedNode;

class CompletionTrie {
public:
	CompletionTrie();
	virtual ~CompletionTrie();

	void addTerm(const std::string term, const u_int32_t score);
	std::vector<PackedNode*> findLocus(const std::string term,
			bool& return_foundTerm);
	SimpleSuggestions* getSuggestions(const std::string prefix, const int k);
	char* mem;
private:
	static const long initialMemSize = 80; //1 << 20;

	/*
	 * The main memory used to store the trie
	 * TODO: Use a memory mapped file instead
	 */

	long memSize;

	u_int64_t firstNodePointer;
	u_int64_t firstFreeMemPointer;

	/**
	 * Shifts all Nodes right of node to the right by width bytes and updates the
	 * firstChildOffset of all left siblings
	 *
	 * @return A pointer to the first free byte (with width following free bytes)
	 */
	u_int64_t makeRoomBehindNode(PackedNode* node, PackedNode* parent,
			const uint width);

	PackedNode* findLeftSibling(const u_int32_t deltaScore, PackedNode* parent);

	u_int64_t getPositionInMem(const PackedNode* node) {
		return ((long) node) - ((long) mem);
	}
};

#endif /* COMPLETIONTRIE_H_ */
