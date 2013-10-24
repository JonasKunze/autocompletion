//============================================================================
// Name        : main.cpp
// Author      : Jonas Kunze
// Version     :
// Copyright   : GPLv3
// Description : Main method of the autocompletion service
//============================================================================

#include <iostream>

#include "CompletionTrie.h"
#include "PackedNode.h"

using namespace std;

struct test {
	char chars[];
	char chars2[];
};

int main() {
	CompletionTrie trie;
	trie.addTerm("asdf123", 1234);
	trie.addTerm("asdf1234", 1234);

	bool return_foundTerm;
	std::vector<PackedNode*> locus = trie.findLocus("asdf123abc",
			return_foundTerm);
	std::cout
			<< std::string(locus.back()->getCharacters(),
					locus.back()->charactersSize_) << std::endl;
	return 0;
}
