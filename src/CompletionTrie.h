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
private:
	static const long initialMemSize = 1 << 20;

	/*
	 * The main memory used to store the trie
	 * TODO: Use a memory mapped file instead
	 */
	char* mem;
	long memSize;

	u_int64_t firstNodePointer;
	u_int64_t lastNodePointer;
};

#endif /* COMPLETIONTRIE_H_ */
