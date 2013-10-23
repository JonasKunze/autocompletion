/*
 * Node.h
 *
 *  Created on: Oct 23, 2013
 *      Author: Jonas Kunze
 */

//#include <string.h>
#include <cstring>
//#include <iostream>
//#include <string>

#ifndef NODE_H_
#define NODE_H_

#define NUMBER_OF_BYTES_TO_STORE(i) i>>8>0?

/**
 * Returns the number of bytes needed to store the given integer with the following coding:
 * 0: 0 bytes
 * 1: 1 byte
 * 2: 2 bytes
 * 3: 4 bytes
 */
unsigned inline char getNumberOfBytesToStore2b(const int i) {
//	return i == 0 ? 0 : i < (1 << 8) ? 1 : i < (1 << 16) ? 2 : 4;
	int msb;
	asm("bsrl %1,%0" : "=r"(msb) : "r"(i));
	return msb > 15 ? 4 : msb / 8 + 1;
}

/**
 * A compact completion trie node
 */
struct PackedNode {
	/*
	 * Number of characters stored in that node
	 */
	unsigned int charactersSize_ :3;

	/*
	 * Is this Node the last sibling?
	 */
	unsigned int isEndOfWord_ :1;

	/*
	 * Number of bytes used for the delta score value:
	 * 0: 0 bytes
	 * 1: 1 byte
	 * 2: 2 bytes
	 * 3: 4 bytes
	 */
	unsigned int deltaScoreSize_ :2;

	/*
	 *
	 * Number of bytes used for the first child pointer
	 */
	unsigned int firstChildOffsetSize_ :2;

	/*
	 * Stores the characters of this node in the trie, the score difference to the parent
	 * node and the relative pointer to the first child.
	 */
	char characters_deltaScore_firstChildOffset_[];

	/**
	 * Returns the pointer to the character array
	 */
	char* getCharacters() {
		/*
		 * We can return characters_deltaScore_firstChildOffset_ as is at it begins with the characters
		 */
		return characters_deltaScore_firstChildOffset_;
	}

	/**
	 * Returns the pointer to the character array
	 */
	int getDeltaScore() {
		/*
		 * Cast the storage to an array directly behind the characters and use a bitmap to
		 * only return as many bytes as are used for the delta score
		 */
		return *(reinterpret_cast<int*>(characters_deltaScore_firstChildOffset_
				+ charactersSize_)) & ((1 << deltaScoreSize_ * 8) - 1);
	}

	/**
	 * Returns the pointer to the character array
	 */
	int getFirstChildOffset() {
		/*
		 * Cast the storage to an array at the position behind the delta score and use a bitmap to
		 * only return as many bytes as are used for the first child offset
		 */
		return *(reinterpret_cast<int*>(characters_deltaScore_firstChildOffset_
				+ charactersSize_ + deltaScoreSize_))
				& ((1 << firstChildOffsetSize_ * 8) - 1);
	}

	int getSize() const {
		return sizeof(PackedNode) + charactersSize_ + deltaScoreSize_
				+ firstChildOffsetSize_;
	}

}__attribute__((packed));

static PackedNode* createNode(const char characterSize, const char* characters,
		const bool isEndOfWord, const int deltaScore,
		const int firstChildOffset) {

	const char deltaScoreSize = getNumberOfBytesToStore2b(deltaScore);
	const char firstChildOffsetSize = getNumberOfBytesToStore2b(
			firstChildOffset);

	PackedNode *node = static_cast<PackedNode *>(std::malloc(
			sizeof(PackedNode) + characterSize + deltaScoreSize
					+ firstChildOffsetSize));

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

#endif /* NODE_H_ */
