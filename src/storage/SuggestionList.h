/*
 * SuggestionList.h
 *
 *  Created on: Nov 10, 2013
 *      Author: Jonas Kunze
 */

#ifndef SUGGESTIONLIST_H_
#define SUGGESTIONLIST_H_

#include <sys/types.h>
#include <string>
#include <vector>

struct NodeWithRelativeScoreStore;

class SuggestionStore;
struct Suggestion {
	std::string suggestion;
	u_int32_t relativeScore;
	std::string additionalData;
};

struct SuggestionList {
	SuggestionStore* store;
	/**
	 * Suggestion containing only a String
	 */
	std::vector<Suggestion> suggestedWords;

	const u_int8_t k;
	SuggestionList(const u_int8_t _k, SuggestionStore* _store) :
			store(_store), suggestedWords(), k(_k) {
	}

	virtual ~SuggestionList() {
	}

	void addSuggestion(NodeWithRelativeScoreStore nodeWithParent);

	inline bool isFull() const {
		return suggestedWords.size() == k;
	}
};

#endif /* SUGGESTIONLIST_H_ */
