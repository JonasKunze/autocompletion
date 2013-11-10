/*
 * SuggestionList.h
 *
 *  Created on: Nov 10, 2013
 *      Author: Jonas Kunze
 */

#include "SuggestionList.h"

#include "CompletionTrie.h"
#include "PackedNode.h"
#include "SuggestionStore.h"

void SuggestionList::addSuggestion(NodeWithRelativeScoreStore nodeWithParent) {
	const std::string URL = store->getURL(nodeWithParent.node);
	suggestedWords.push_back(
			{ nodeWithParent.getString(), nodeWithParent.getRelativeScore(), URL });
}

void SuggestionList::addSuggestionWithImage(
		NodeWithRelativeScoreStore nodeWithParent) {
	const std::string URL = store->getURL(nodeWithParent.node);
	const std::string image = store->getImage(nodeWithParent.node);

	suggestedWords.push_back(
			{ nodeWithParent.getString(), nodeWithParent.getRelativeScore(),
					URL, image });
}
