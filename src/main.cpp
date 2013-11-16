//============================================================================
// Name        : main.cpp
// Author      : Jonas Kunze
// Version     :
// Copyright   : GPLv3
// Description : Main method of the autocompletion service
//============================================================================

//#include <cstring>

//#include <deque>

#include <ctime>
#include <iostream>
#include <string>
#include <thread>

#include "CompletionServer.h"
#include "CompletionTrie.h"
#include "CompletionTrieBuilder.h"
#include "options/Options.h"
#include "PerformanceTest.h"
#include "SuggestionList.h"
#include "utils/Utils.h"

using namespace std;

void interactiveThread(const CompletionTrie* trie) {
	do {
		std::string str;
		std::cout << "Please enter search string: ";
		std::cin >> str;

		if (str == "\\q") {
			return;
		}

		long start = Utils::getCurrentMicroSeconds();
		std::shared_ptr<SuggestionList> suggestions = trie->getSuggestions(str,
				10);
		long time = Utils::getCurrentMicroSeconds() - start;
		std::cout << time << " us for finding suggestions" << std::endl;

		for (Suggestion sugg : suggestions->suggestedWords) {
			std::cout << sugg.suggestion << "\t" << sugg.relativeScore << "\t"
					<< sugg.image << std::endl;
		}
	} while (true);
}
int main(int argc, char* argv[]) {
	Options::Initialize(argc, argv);

	CompletionTrie* trie = CompletionTrieBuilder::buildFromFile(
			Options::GetString(OPTION_LOAD_FILE));

//	trie->print();

//	PerformanceTest::runTest(trie);

//	std::thread t(&interactiveThread, trie);
//
	CompletionServer server(trie);
	server.start();

	return 0;
}
