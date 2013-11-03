//============================================================================
// Name        : main.cpp
// Author      : Jonas Kunze
// Version     :
// Copyright   : GPLv3
// Description : Main method of the autocompletion service
//============================================================================

//#include <cstring>

//#include <deque>
#include<iostream>
#include<fstream>
#include <memory>
#include <string>

#include "CompletionTrie.h"
#include "CompletionTrieBuilder.h"
#include "Suggestion.h"
#include "Utils.h"

using namespace std;

static std::vector<std::pair<std::string, int> > readFile(
		std::string fileName) {
	std::vector<std::pair<std::string, int> > nodes;
	ifstream myReadFile;
	myReadFile.open(fileName);
	if (myReadFile.is_open()) {
		while (!myReadFile.eof()) {
			std::string term;
			u_int32_t score;

			myReadFile >> term;
			myReadFile >> score;

			nodes.push_back(std::make_pair(term, score));
		}
	}
	myReadFile.close();
	return nodes;
}

int main() {
	CompletionTrieBuilder builder;

	long start = getCurrentMicroSeconds();
	std::vector<std::pair<std::string, int> > nodeValues = readFile(
			"data/wiki-100000.tsv");
	long time = getCurrentMicroSeconds() - start;
	std::cout << time / 1000l << " ms for reading file" << std::endl;

	start = getCurrentMicroSeconds();
	for (auto nodeValue : nodeValues) {
		builder.addString(nodeValue.first, nodeValue.second);
	}
	time = getCurrentMicroSeconds() - start;
	std::cout << time / 1000l << " ms for creating builder trie" << std::endl;

//	builder.addString("abc", 1235);
//	builder.addString("abd", 1236);
//	builder.addString("ab", 1236);

	CompletionTrie* trie = builder.generateCompletionTrie();

//	builder.print();

//	trie->print();

//	std::shared_ptr<SimpleSuggestions> suggestions = trie->getSuggestions("b",
//			5);
//	for (std::string s : suggestions->suggestedWords) {
//		std::cout << s << std::endl;
//	}

//
//	bool return_foundTerm;
//	CompletionTrie trie;
//	trie.addTerm("12345", 1234);
//	trie.print();
//	std::cout << std::endl;
//	trie.addTerm("123456", 1234);
//	trie.print();
//	std::cout << std::endl;
//	trie.addTerm("abcd", 1234);
//	trie.print();
//	std::cout << std::endl;
//	trie.addTerm("abcd1", 1234);
//	trie.print();
//	std::cout << std::endl;
//	trie.addTerm("abcd12", 1234);
//	trie.print();
//	std::cout << std::endl;
//	trie.addTerm("123457", 1234);
//	trie.print();
//	std::cout << std::endl;
//
//	std::cout << trie.findLocus("asdf123", return_foundTerm).size() << " : ";
//	std::cout << return_foundTerm << std::endl;

	return 0;
}
