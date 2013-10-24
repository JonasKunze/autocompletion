/*
 * Suggestion.h
 *
 *  Created on: Oct 23, 2013
 *      Author: Jonas Kunze
 */

#ifndef SUGGESTION_H_
#define SUGGESTION_H_

#include <string>

/**
 * Suggestion containing only a String
 */
struct SimpleSuggestions {
	std::string* suggestedWords;

	SimpleSuggestions(const int k) {
		suggestedWords = new std::string[k];
	}
	~SimpleSuggestions() {
		delete[] suggestedWords;
	}
};

#endif /* SUGGESTION_H_ */
