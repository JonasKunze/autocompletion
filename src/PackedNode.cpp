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
		const char* characters, const bool isLastSibling, const int deltaScore,
		const int firstChildOffset) {

	char* memory = new char[getMaxSize()];
	return createNode(memory, 0, characterSize, characters, isLastSibling,
			deltaScore, firstChildOffset);
}

PackedNode* PackedNode::createRootNode(char* memory) {
	return createNode(memory, 0, 0, 0, false, 0,
			sizeof(PackedNode) + 1 /*1 byte to store child offset*/);
}

PackedNode* PackedNode::createNode(char* memory, u_int64_t memPointer,
		const char characterSize, const char* characters,
		const bool isLastSibling, const int deltaScore,
		const int firstChildOffset) {

	const char deltaScoreSize = getNumberOfBytesToStore2b(deltaScore);

	PackedNode *node;
	node = reinterpret_cast<PackedNode *>(memory + memPointer);

	node->charactersSize_ = characterSize;
	node->isLastSibling = isLastSibling;
	node->deltaScoreSize_ = deltaScoreSize;

	memcpy(node->characters_deltaScore_firstChildOffset_, characters,
			characterSize);
	memcpy(node->characters_deltaScore_firstChildOffset_ + characterSize,
			&deltaScore, deltaScoreSize);

	node->setFirstChildOffset(firstChildOffset);
	return node;
}
