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
//	char* c1 = "abcd";
//	char* c2 = "abed";
//	u_int64_t l1 = *((u_int64_t*) c1);
//	u_int64_t l2 = *((u_int64_t*) c2);
//	u_int64_t k = l1 ^ l2;
//
//	int r = ffsl(k);
//	std::cout << ((r > 0) ? 1 + (r - 1) / 8 : 0) << std::endl;

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
	std::cout << std::endl;
	trie.addTerm("abcd12", 1234);
	trie.print();
	std::cout << std::endl;
	trie.addTerm("123457", 1234);
	trie.print();
	std::cout << std::endl;

	std::cout << trie.findLocus("asdf123", return_foundTerm).size() << " : ";
	std::cout << return_foundTerm << std::endl;

	return 0;
}
