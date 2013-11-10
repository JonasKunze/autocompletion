/*
 * SuggestionStore.h
 *
 *  Created on: Nov 10, 2013
 *      Author: Jonas Kunze
 */

#ifndef SUGGESTIONSTORE_H_
#define SUGGESTIONSTORE_H_

#include <sys/types.h>
#include <iostream>
#include <unordered_map>
#include <memory>
#include <string>

#include "PackedNode.h"
#include "SuggestionList.h"

class PackedNode;

struct ImageAndURL {
	std::string image;
	std::string URL;
};

class SuggestionStore {
private:
	/*
	 * If the nodes have been moved since addTerm() has been called,
	 * the delta will be stored here
	 */
	u_int64_t pointerMovedDelta;

	/*
	 * images and URLs by (u_int64_t)node where node is PackedNode*
	 *
	 */
	std::unordered_map<u_int64_t, ImageAndURL> imagesAndURLs;

public:
	SuggestionStore() :
			pointerMovedDelta(0) {
	}

	virtual ~SuggestionStore() {
	}

	std::shared_ptr<SuggestionList> getSuggestionList(const u_int8_t k) {
		return std::make_shared<SuggestionList>(k, this);
	}

	void addTerm(PackedNode* node, std::string URL, std::string image) {
		imagesAndURLs[reinterpret_cast<u_int64_t>(node)] = {image, URL};
	}

	ImageAndURL getImageAndURL(PackedNode* node) {
		return imagesAndURLs[reinterpret_cast<u_int64_t>(node) + pointerMovedDelta];
	}

	void setPointerDelta(const u_int64_t delta) {
		pointerMovedDelta = delta;
	}
};

#endif /* SUGGESTIONSTORE_H_ */
