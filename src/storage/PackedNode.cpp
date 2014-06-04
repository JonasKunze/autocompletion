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
	return createNode(memory, characterSize, characters, isLastSibling,
			deltaScore, firstChildOffset);
}

PackedNode* PackedNode::createRootNode(char* memory) {
	return createNode(memory, 0, 0, true, 0,
			sizeof(PackedNode) + 1 /*1 byte to store child offset*/);
}

PackedNode* PackedNode::createNode(char* memory, const char characterSize,
		const char* characters, const bool isLastSibling,
		const u_int32_t deltaScore, const int firstChildOffset) {

	const u_int8_t deltaScoreSize = getNumberOfBytesToStore2b(deltaScore);

	PackedNode *node;
	node = reinterpret_cast<PackedNode *>(memory);

	node->charactersSize_ = characterSize;
	node->isLastSibling_ = isLastSibling;
	node->deltaScoreSize_ = deltaScoreSize;

	memcpy(node->characters_deltaScore_firstChildOffset_, characters,
			characterSize);
	memcpy(node->characters_deltaScore_firstChildOffset_ + characterSize,
			&deltaScore, numberOfBytesBy2bValue[deltaScoreSize]);

	node->setFirstChildOffset(firstChildOffset);
	return node;
}
