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
#include <set>

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
	 * images and URIs by (u_int64_t)node where node is PackedNode*
	 */
	std::unordered_map<u_int64_t, std::string> dataByNode;

public:
	SuggestionStore() :
			pointerMovedDelta(0) {
	}

	virtual ~SuggestionStore() {
	}

	std::shared_ptr<SuggestionList> getSuggestionList(const u_int8_t k) {
		return std::make_shared < SuggestionList > (k, this);
	}

	void addTerm(PackedNode* node, std::string additionalData);

	const std::string getAdditionalData(PackedNode* node) {
		return dataByNode[reinterpret_cast<u_int64_t>(node)
				+ pointerMovedDelta];
	}

	void setPointerDelta(const u_int64_t delta) {
		pointerMovedDelta = delta;
	}
};

#endif /* SUGGESTIONSTORE_H_ */
