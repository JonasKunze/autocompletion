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
	bool return_foundTerm;
	CompletionTrie trie;
	trie.addTerm("12345", 1234);
	trie.print();
	std::cout << std::endl;
	trie.addTerm("123456", 1234);
	trie.print();
	std::cout << std::endl;
	trie.addTerm("abcd", 1234);
	trie.print();
	std::cout << std::endl;
	trie.addTerm("abcd1", 1234);
	trie.print();

	std::cout << trie.findLocus("asdf123", return_foundTerm).size() << " : ";
	std::cout << return_foundTerm << std::endl;

	return 0;
}
