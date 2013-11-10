/*
 * SuggestionStore.h
 *
 *  Created on: Nov 10, 2013
 *      Author: Jonas Kunze
 */

#ifndef SUGGESTIONSTORE_H_
#define SUGGESTIONSTORE_H_

#include <sys/types.h>
#include <map>
#include <memory>
#include <string>
#include "SuggestionList.h"

class SuggestionStore {
private:
	/*
	 * images by term+leafNodeDeltaScore
	 *
	 * This way we can have several times the same term with different images
	 */
	std::map<std::string, std::string> images;

	/*
	 * URLs by term+leafNodeDeltaScore
	 *
	 * This way we can have several times the same term with different images
	 */
	std::map<std::string, std::string> URLs;
public:
	SuggestionStore() {
	}

	virtual ~SuggestionStore() {
	}

	std::shared_ptr<SuggestionList> getSuggestionList(const u_int8_t k) {
		return std::make_shared<SuggestionList>(k, this);
	}

	void addTerm(std::string term, u_int32_t leafNodeDeltaScore,
			std::string URL, std::string image) {
		images[term + std::to_string(leafNodeDeltaScore)] = image;
		URLs[term + std::to_string(leafNodeDeltaScore)] = URL;
	}

	void addTerm(std::string term, u_int32_t leafNodeDeltaScore,
			std::string URL) {
		URLs[term + std::to_string(leafNodeDeltaScore)] = URL;
	}

	std::string getURL(std::string term, u_int32_t leafNodeDeltaScore) {
		return URLs[term + std::to_string(leafNodeDeltaScore)];
	}

	std::string getImage(std::string term, u_int32_t leafNodeDeltaScore) {
		return images[term + std::to_string(leafNodeDeltaScore)];
	}
};

#endif /* SUGGESTIONSTORE_H_ */
