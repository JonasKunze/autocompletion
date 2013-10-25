/*
 * PackedNode.h
 *
 *  Created on: Oct 23, 2013
 *      Author: Jonas Kunze
 */

#ifndef PACKEDNODE_H_
#define PACKEDNODE_H_

#include <sys/types.h>
#include <cstring>

/**
 * Returns the number of bytes needed to store the given integer with the following coding:
 * 0: 0 bytes
 * 1: 1 byte
 * 2: 2 bytes
 * 3: 4 bytes
 *
 * TODO: This takes about 9µs and could be optimized, but it's not used too often
 */
inline u_int8_t getNumberOfBytesToStore2b(const int i) {
//	return i == 0 ? 0 : i < (1 << 8) ? 1 : i < (1 << 16) ? 2 : 4;
	int msb;
	asm("bsrl %1,%0" : "=r"(msb) : "r"(i));
	return msb == 0 ? 0 : msb <= 24 ? msb / 8 + 1 : 3;
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
	unsigned int isLastSibling :1;

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

	///////// FIRST BYTE DONE

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
	u_int32_t getDeltaScore() {
		/*
		 * Cast the storage to an array directly behind the characters and use a bitmap to
		 * only return as many bytes as are used for the delta score
		 */
		return *(reinterpret_cast<int*>(characters_deltaScore_firstChildOffset_
				+ charactersSize_)) & ((1 << deltaScoreSize_ * 8) - 1);
	}

	/**
	 * Returns the pointer to the character array (counted from the first byte of this node)
	 */
	u_int32_t getFirstChildOffset() {
		/*
		 * Cast the storage to an array at the position behind the delta score and use a bitmap to
		 * only return as many bytes as are used for the first child offset
		 */
		return *(reinterpret_cast<int*>(characters_deltaScore_firstChildOffset_
				+ charactersSize_ + deltaScoreSize_))
				& ((1 << firstChildOffsetSize_ * 8) - 1);
	}

	/**
	 * Returns the pointer to the character array (counted from the first byte of this node)
	 */
	u_int32_t setFirstChildOffset(const u_int32_t offset) {
		firstChildOffsetSize_ = getNumberOfBytesToStore2b(offset);
		memcpy(
				characters_deltaScore_firstChildOffset_ + charactersSize_
						+ deltaScoreSize_, &offset, firstChildOffsetSize_);
	}

	/**
	 * Returns the number of bytes this node has to be extended if the firstChildOffset would be
	 * changed to the given value
	 */
	u_int8_t bytesToExtendOnFirstChildOffsetChange(const uint offset) {
		return getNumberOfBytesToStore2b(getFirstChildOffset() + offset)
				- firstChildOffsetSize_;
	}

	int getSize() const {
		return sizeof(PackedNode) + charactersSize_ + deltaScoreSize_
				+ firstChildOffsetSize_;
	}

	/**
	 * Returns the maximum size a PackedNode could take
	 */
	static u_int32_t getMaxSize() {
		return sizeof(PackedNode) + 7 + 4 + 3;
	}

	/**
	 * Creates a new PackedNode with it's own allocated memory
	 */
	static PackedNode* createNode(const char characterSize,
			const char* characters, const bool isEndOfWord,
			const int deltaScore, const int firstChildOffset);

	/**
	 * Creates a new root Node at the beginning of the given memory
	 */
	static PackedNode* createRootNode(char* memory);

	/**
	 * Creates a new PackedNode inside the given memory
	 *
	 * @param floatLeft If true the last byte of the new Node will be memPointer, If false
	 * this pointer will be the first byte
	 */
	static PackedNode* createNode(char* memory,
			u_int64_t firstBlockedByteInMemoryPointer, const char characterSize,
			const char* characters, const bool isEndOfWord,
			const int deltaScore, const int firstChildOffset);

}__attribute__((packed));

#endif /* PACKEDNODE_H_ */
