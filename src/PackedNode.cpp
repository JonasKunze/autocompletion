/*
 * PackedNode.cpp
 *
 *  Created on: Oct 23, 2013
 *      Author: Jonas Kunze
 */

#include "PackedNode.h"
#include <stdlib.h>
#include <cstring>

PackedNode* PackedNode::createNode(const char characterSize,
		const char* characters, const bool isEndOfWord, const int deltaScore,
		const int firstChildOffset) {

	char* memory = new char[getMaxSize()];
	return createNode(memory, 0, characterSize, characters, isEndOfWord,
			deltaScore, firstChildOffset, false);
}

PackedNode* PackedNode::createNode(void* memory, u_int64_t memPointer,
		const char characterSize, const char* characters,
		const bool isEndOfWord, const int deltaScore,
		const int firstChildOffset, bool floatLeft) {

	const char deltaScoreSize = getNumberOfBytesToStore2b(deltaScore);
	const char firstChildOffsetSize = getNumberOfBytesToStore2b(
			firstChildOffset);

	PackedNode *node;
	if (floatLeft) {
		node = static_cast<PackedNode *>(memory + memPointer
				- (sizeof(PackedNode) + characterSize + deltaScoreSize
						+ firstChildOffsetSize) + 1);
	} else {
		node = static_cast<PackedNode *>(memory + memPointer);
	}

	node->charactersSize_ = characterSize;
	node->isEndOfWord_ = isEndOfWord;
	node->deltaScoreSize_ = deltaScoreSize;
	node->firstChildOffsetSize_ = firstChildOffsetSize;

	memcpy(node->characters_deltaScore_firstChildOffset_, characters,
			characterSize);
	memcpy(node->characters_deltaScore_firstChildOffset_ + characterSize,
			&deltaScore, deltaScoreSize);
	memcpy(
			node->characters_deltaScore_firstChildOffset_ + characterSize
					+ deltaScoreSize, &firstChildOffset, firstChildOffsetSize);
	return node;
}
