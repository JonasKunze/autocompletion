/*
 * Suggestion.h
 *
 *  Created on: Oct 23, 2013
 *      Author: Jonas Kunze
 */

#ifndef SUGGESTION_H_
#define SUGGESTION_H_

#include <string>
#include <vector>

/**
 * Suggestion containing only a String
 */
struct SimpleSuggestions {
	std::vector<std::pair<std::string, u_int32_t>> suggestedWords;
	const u_int8_t k;
	SimpleSuggestions(const u_int8_t _k) :
			suggestedWords(), k(_k) {
	}

	~SimpleSuggestions() {
	}

	inline void addSuggestion(std::string suggestion, u_int32_t score) {
		suggestedWords.push_back(std::make_pair(suggestion, score));
	}

	inline bool isFull() const {
		return suggestedWords.size() == k;
	}
};

#endif /* SUGGESTION_H_ */
