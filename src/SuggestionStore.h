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
#include <map>
#include <memory>
#include <string>

#include "PackedNode.h"
#include "SuggestionList.h"

class PackedNode;

class SuggestionStore {
private:
	/*
	 * If the nodes have been moved since addTerm() has been called,
	 * the delta will be stored here
	 */
	u_int64_t pointerMovedDelta;

	/*
	 * images by (u_int64_t)node where node is PackedNode*
	 *
	 */
	std::map<u_int64_t, std::string> images;

	/*
	 * URLs by (u_int64_t)node where node is PackedNode*
	 */
	std::map<u_int64_t, std::string> URLs;
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
		images[reinterpret_cast<u_int64_t>(node)] = image;
		URLs[reinterpret_cast<u_int64_t>(node)] = URL;
	}

//	void addTerm(PackedNode* node, std::string* URL) {
//		if (URL != nullptr) {
//			URLs[reinterpret_cast<u_int64_t>(node)] = std::string(*URL);
//		}
//	}

	std::string getURL(PackedNode* node) {
		return URLs[reinterpret_cast<u_int64_t>(node) + pointerMovedDelta];
	}

	std::string getImage(PackedNode* node) {
		return images[reinterpret_cast<u_int64_t>(node) + pointerMovedDelta];
	}

	void setPointerDelta(const u_int64_t delta) {
		pointerMovedDelta = delta;
	}
};

#endif /* SUGGESTIONSTORE_H_ */
