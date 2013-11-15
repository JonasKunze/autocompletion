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
	const ImageAndURI* imageAndURI = store->getImageAndURI(nodeWithParent.node);
	suggestedWords.push_back(
			{ nodeWithParent.getString(), nodeWithParent.getRelativeScore(),
					imageAndURI->URI });
}

void SuggestionList::addSuggestionWithImage(
		NodeWithRelativeScoreStore nodeWithParent) {
	const ImageAndURI* imageAndURI = store->getImageAndURI(nodeWithParent.node);

	suggestedWords.push_back(
			{ nodeWithParent.getString(), nodeWithParent.getRelativeScore(),
					imageAndURI->URI, imageAndURI->image });
}
