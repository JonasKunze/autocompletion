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

std::vector<std::pair<std::string, int> > readFile(std::string fileName) {
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

void performanceTest() {
	CompletionTrieBuilder builder;
	long start = Utils::getCurrentMicroSeconds();
	std::vector<std::pair<std::string, int> > nodeValues = readFile(
			"data/wiki-10000.tsv");
	long time = Utils::getCurrentMicroSeconds() - start;
	std::cout << time / 1000l << " ms for reading file" << std::endl;

	start = Utils::getCurrentMicroSeconds();
	for (auto nodeValue : nodeValues) {
		builder.addString(nodeValue.first, nodeValue.second);
	}
	time = Utils::getCurrentMicroSeconds() - start;
	std::cout << time / 1000l << " ms for creating builder trie" << std::endl;

	CompletionTrie* trie = builder.generateCompletionTrie();
	builder.print();

	std::shared_ptr<SimpleSuggestions> suggestions = trie->getSuggestions("'",
			10);

	for (std::string str : suggestions->suggestedWords) {
		std::cout << str << std::endl;
	}

	const char* chars =
			(char*) "'.-_+0123456789abcdefghijklmnopqrstuvwxyz'.-_+0123456789abcdefghijklmnopqrstuvwxyz";

	start = Utils::getCurrentMicroSeconds();
	for (int i = 0; i < 1000; i++) {
		int pos = std::rand() * (1.0 / (RAND_MAX + 1.0)) * 41;
		std::string randStr = std::string(&chars[pos], 3);
		trie->getSuggestions(randStr, 10);
	}
	time = Utils::getCurrentMicroSeconds() - start;
	std::cout << time / 1000 << " us for finding suggestions" << std::endl;

	do {
		std::string str;
		std::cout << "Please enter search string: ";
		std::cin >> str;

		if (str == "\\q") {
			return;
		}

		start = Utils::getCurrentMicroSeconds();
		trie->getSuggestions(str, 10);
		time = Utils::getCurrentMicroSeconds() - start;
		std::cout << time << " us for finding suggestions" << std::endl;

	} while (true);

}

int main() {
//	CompletionTrieBuilder builder;
//
//	builder.addString("abcd", 1235);
//	builder.addString("abd", 1236);
//	builder.addString("abe", 1236);
//	builder.addString("ab", 1236);
//	builder.addString("a", 1236);
//	builder.addString("b", 1236);
//	builder.addString("bcd", 1236);
//	builder.addString("bcdef", 1236);
//
//	CompletionTrie* trie = builder.generateCompletionTrie();
//	trie->print();
//
//	std::shared_ptr<SimpleSuggestions> suggestions = trie->getSuggestions("a",
//			10);
//
//	for (std::string str : suggestions->suggestedWords) {
//		std::cout << str << std::endl;
//	}

	performanceTest();

	return 0;
}
