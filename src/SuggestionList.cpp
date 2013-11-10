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
	const std::string term = nodeWithParent.getString();
	const std::string URL = store->getURL(term,
			nodeWithParent.node->getDeltaScore());
	suggestedWords.push_back( { term, nodeWithParent.getRelativeScore(), URL });
}

void SuggestionList::addSuggestionWithImage(
		NodeWithRelativeScoreStore nodeWithParent) {
	const std::string term = nodeWithParent.getString();
	const std::string URL = store->getURL(term,
			nodeWithParent.node->getDeltaScore());
	const std::string image = store->getImage(term,
			nodeWithParent.node->getDeltaScore());

	suggestedWords.push_back( { term, nodeWithParent.getRelativeScore(), URL,
			image });
}
