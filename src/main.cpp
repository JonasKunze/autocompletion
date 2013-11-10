//============================================================================
// Name        : main.cpp
// Author      : Jonas Kunze
// Version     :
// Copyright   : GPLv3
// Description : Main method of the autocompletion service
//============================================================================

//#include <cstring>

//#include <deque>

#include <sys/types.h>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "CompletionTrie.h"
#include "CompletionTrieBuilder.h"
#include "SuggestionList.h"
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
//			"data/all.1gs");
//			"data/test.tsv");
	long time = Utils::getCurrentMicroSeconds() - start;
	std::cout << time / 1000. << " ms for reading file" << std::endl;

	start = Utils::getCurrentMicroSeconds();

	for (auto nodeValue : nodeValues) {
		builder.addString(nodeValue.first, nodeValue.second, nodeValue.first,
				nodeValue.first);
	}
	time = Utils::getCurrentMicroSeconds() - start;
	std::cout << time / 1000. << " ms for creating builder trie" << std::endl;

	std::cout << "Total memory consumption: " << Utils::GetMemUsage() / 1000000.
			<< std::endl;

	start = Utils::getCurrentMicroSeconds();
	CompletionTrie* trie = builder.generateCompletionTrie();

	time = Utils::getCurrentMicroSeconds() - start;
	std::cout << time / 1000. << " ms for creating packed trie with "
			<< trie->getMemoryConsumption() << std::endl;

	std::cout << "Total memory consumption: " << Utils::GetMemUsage() / 1000000.
			<< std::endl;

//	trie->print();

	std::shared_ptr<SuggestionList> suggestions = trie->getSuggestions("'", 10);

	std::cout << "Found " << suggestions->suggestedWords.size()
			<< " suggestions:" << std::endl;
	for (Suggestion sugg : suggestions->suggestedWords) {
		std::cout << sugg.suggestion << "\t" << sugg.relativeScore << "\t"
				<< sugg.URL << "\t" << sugg.image << std::endl;
	}

	const char* chars = (char*) "'.-_+01234";

	start = Utils::getCurrentMicroSeconds();
	int runs = 100000;
	for (int i = 0; i < runs; i++) {
		int pos = std::rand() * (1.0 / (RAND_MAX + 1.0)) * 10;
		std::string randStr = std::string(&chars[pos], 6);
		std::shared_ptr<SuggestionList> suggestions = trie->getSuggestions(
				randStr, 10);
	}
	time = Utils::getCurrentMicroSeconds() - start;
	std::cout << time / (float) runs << " us for finding suggestions"
			<< std::endl;

//	do {
//		std::string str;
//		std::cout << "Please enter search string: ";
//		std::cin >> str;
//
//		if (str == "\\q") {
//			return;
//		}
//
//		start = Utils::getCurrentMicroSeconds();
//		std::shared_ptr<SuggestionList> suggestions = trie->getSuggestions(str,
//				10);
//		time = Utils::getCurrentMicroSeconds() - start;
//		std::cout << time << " us for finding suggestions" << std::endl;
//
//		for (Suggestion sugg : suggestions->suggestedWords) {
//			std::cout << sugg.suggestion << "\t" << sugg.relativeScore << "\t"
//					<< sugg.image << std::endl;
//		}
//	} while (true);
}

int main() {
//	CompletionTrieBuilder builder;
//	builder.addString("a", 15078, std::string("image"), std::string("url"));
//	builder.addString("a", 13132, "image", "url");
//
//////
//////	builder.addString("'Outstanding", 175);
//////	builder.addString("'Operation", 141);
//////	builder.addString("'Open", 92);
//////	builder.addString("'", 92);
//////	builder.print();
//////
//////	builder.addString("abcdefg", 1235);
//////	builder.addString("a1234567b	", 1236);
//////	builder.addString("ab", 1236);
//////
//////	builder.addString("abcdefgh", 1235);
//////	builder.addString("abcdefgh", 1236);
//////
//////	builder.addString("abc", 1235);
//////	builder.addString("abc", 1236);
//////
//////	builder.addString("a", 1235);
//////	builder.addString("a", 1236);
//////
//////	builder.addString("abcd", 1235);
//////	builder.addString("a", 1236);
//////	builder.addString("ae", 1236);
//////	builder.addString("afcd", 1235);
//////
//////	builder.addString("a", 1235);
//////	builder.addString("abc", 1236);
//////	builder.addString("ade", 1236);
//////
//////	builder.addString("abc", 1236);
//////	builder.addString("abe", 1235);
//////	builder.addString("ade", 1236);
//
//	CompletionTrie* trie = builder.generateCompletionTrie();
//	builder.print();
//
//	trie->print();
//	std::shared_ptr<SuggestionList> suggestions = trie->getSuggestions("a", 10);
//
//	for (Suggestion sugg : suggestions->suggestedWords) {
//		std::cout << sugg.suggestion << "\t" << sugg.relativeScore << "\t"
//				<< sugg.URL << "\t" << sugg.image << std::endl;
//	}
//
	performanceTest();

	return 0;
}
