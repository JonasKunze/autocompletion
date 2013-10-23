/*
 * Node.h
 *
 *  Created on: Oct 23, 2013
 *      Author: Jonas Kunze
 */

#include <string.h>

#ifndef NODE_H_
#define NODE_H_

#define NUMBER_OF_BYTES_TO_STORE(i) i>>8>0?

static const unsigned char getNumberOfBytesToStore(int i) {
	unsigned char byteNum = 0;
	while (i > 0) {
		byteNum++;
		i >>= 8;
	}
	return byteNum;
	return 1;
}

/**
 * A compact completion trie node
 */
template<unsigned char charactersSize, unsigned char deltaScoreSize,
		unsigned char firstChildOffsetSize>
struct PackedNode {
	/*
	 * Number of characters stored in that node
	 */
	const int charactersSize_ :3;

	/*
	 * Is this Node the last sibling?
	 */
	const bool isEndOfWord_;

	/*
	 * Number of bytes used for the delta score value:
	 * 0: 0 bytes
	 * 1: 1 byte
	 * 2: 2 bytes
	 * 3: 4 bytes
	 */
	const int deltaScoreSize_ :2;

	/*
	 * Number of bytes used for the first child pointer
	 */
	const int firstChildOffsetSize_ :3;
	/*
	 * The characters of this node in the trie.
	 */
	char characters_[charactersSize];

	/*
	 * The delta between the score of the parent node and this node with variable length
	 * (@see scoreSize_)
	 */
	char deltaScore_[deltaScoreSize];
	/*
	 * Number of bytes to jump from here to hit the first child node of this node with
	 * variable length (@see firstChildOffsetSize_)
	 */
	char firstChildOffset_[firstChildOffsetSize];

	PackedNode(const char* characters, bool isEndOfWord, int deltaScore,
			const int firstChildOffset) :
			charactersSize_(charactersSize), isEndOfWord_(isEndOfWord), deltaScoreSize_(
					deltaScoreSize), firstChildOffsetSize_(firstChildOffsetSize) {
		memcpy(characters_, characters, charactersSize);
		memcpy(deltaScore_, &deltaScore, deltaScoreSize);
		memcpy(firstChildOffset_, &firstChildOffset, firstChildOffsetSize);
	}
}__attribute__((packed));

#endif /* NODE_H_ */
