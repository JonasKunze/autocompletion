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
#include <sstream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "CompletionTrie.h"
#include "CompletionTrieBuilder.h"
#include "SuggestionList.h"
#include "utils/Utils.h"
class PerformanceTest {
public:
	static CompletionTrie* runTest(CompletionTrie* trie) {
//	trie->print();


		int runs = 10000;
		long start = Utils::getCurrentMicroSeconds();
		for (int i = 0; i < runs; i++) {
			std::stringstream randStr;
			for (int j = std::rand() * (1.0 / (RAND_MAX + 1.0)) * 6; j != 0;
					j--) {
				int pos = std::rand() * (1.0 / (RAND_MAX + 1.0)) * 26;
				randStr << (char) ('a' + pos);
			}
		}
		long randomTime = Utils::getCurrentMicroSeconds() - start;

		for (int i = 0; i < runs; i++) {
			std::stringstream randStr;
			for (int j = std::rand() * (1.0 / (RAND_MAX + 1.0)) * 6; j != 0;
					j--) {
				int pos = std::rand() * (1.0 / (RAND_MAX + 1.0)) * 26;
				randStr << (char) ('a' + pos);
			}
			std::shared_ptr<SuggestionList> suggestions = trie->getSuggestions(
					randStr.str(), 10);

//			std::cout << randStr.str() << " gave us: " << std::endl;
//			for (Suggestion sugg : suggestions->suggestedWords) {
//				std::cout << sugg.suggestion << "\t" << sugg.relativeScore
//						<< "\t" << sugg.URI << "\t" << sugg.image << std::endl;
//			}
		}
		long time = Utils::getCurrentMicroSeconds() - start - randomTime;
		std::cout << time / (float) runs
				<< " us for finding suggestions (random time "
				<< randomTime / (float) runs << "µs is extracted)" << std::endl;

		return trie;
	}
}
;

#endif /* PERFORMANCETEST_H_ */
