/*
 * PerformanceTest.h
 *
 *  Created on: Nov 12, 2013
 *      Author: Jonas Kunze
 */

#ifndef PERFORMANCETEST_H_
#define PERFORMANCETEST_H_

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
#include "utils/Utils.h"

static CompletionTrie* performanceTest(CompletionTrie* trie) {
//	trie->print();

	std::shared_ptr<SuggestionList> suggestions = trie->getSuggestions("'", 10);

	std::cout << "Found " << suggestions->suggestedWords.size()
			<< " suggestions:" << std::endl;
	for (Suggestion sugg : suggestions->suggestedWords) {
		std::cout << sugg.suggestion << "\t" << sugg.relativeScore << "\t"
				<< sugg.URL << "\t" << sugg.image << std::endl;
	}

	const char* chars = (char*) "'.-_+01234";

	long start = Utils::getCurrentMicroSeconds();
	int runs = 100000;
	for (int i = 0; i < runs; i++) {
		int pos = std::rand() * (1.0 / (RAND_MAX + 1.0)) * 10;
		std::string randStr = std::string(&chars[pos], 6);
		std::shared_ptr<SuggestionList> suggestions = trie->getSuggestions(
				randStr, 10);
	}
	long time = Utils::getCurrentMicroSeconds() - start;
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
	return trie;
}

#endif /* PERFORMANCETEST_H_ */
