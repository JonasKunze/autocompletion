/*
 * SuggestionStore.cpp
 *
 *  Created on: Nov 10, 2013
 *      Author: Jonas Kunze
 */

#include "SuggestionStore.h"

void SuggestionStore::addTerm(PackedNode* node, std::string additionalData) {
	dataByNode[reinterpret_cast<u_int64_t>(node)] = std::move(additionalData);
}
