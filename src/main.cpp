//============================================================================
// Name        : main.cpp
// Author      : Jonas Kunze
// Version     :
// Copyright   : GPLv3
// Description : Main method of the autocompletion service
//============================================================================

//#include <cstring>
#include <iostream>
#include <new>
#include <string>
#include <vector>

#include "CompletionTrie.h"

struct PackedNode;

using namespace std;

int main() {
	CompletionTrie trie;
	trie.addTerm("asdf", 1234);
	trie.addTerm("asdf123", 1234);

	bool return_foundTerm;
	std::vector<PackedNode*> locus = trie.findLocus("asdf", return_foundTerm);
	PackedNode* closestNode = locus.back();

	return 0;
}
