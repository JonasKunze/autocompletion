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
	suggestedWords.push_back(
			{ nodeWithParent.getString(), nodeWithParent.getRelativeScore(),
					store->getAdditionalData(nodeWithParent.node) });
}
