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
#include <string>

#include "../utils/Utils.h"

inline u_int8_t getNumberOfBytesToStore(u_int32_t i) {
	return i == 0 ? 0 : i < (1 << 8) ? 1 : i < (1 << 16) ? 2 :
			i < (1 << 24) ? 3 : 4;
}

/**
 * Returns the number of bytes needed to store the given integer with the following coding:
 * 0: 0 bytes
 * 1: 1 byte
 * 2: 2 bytes
 * 3: 4 bytes
 *
 * TODO: This takes about 9µs and could be optimized, but it's not used too often
 */
inline u_int8_t getNumberOfBytesToStore2b(u_int32_t i) {
	return i == 0 ? 0 : i < (1 << 8) ? 1 : i < (1 << 16) ? 2 : 3;
//	int msb;
//	asm("bsr %1,%0" : "=r"(msb) : "r"(i));
//	return msb == 0 ? 0 : msb <= 24 ? msb / 8 + 1 : 3;
}

/*
 * Some LUTs for the 2bit values
 */
const u_int8_t numberOfBytesBy2bValue[] = { 0, 1, 2, 4 };
const u_int32_t bitmaskFor2bValues[] = { 0, 0xFF, 0xFFFF, 0xFFFFFFFF };

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
	unsigned int isLastSibling_ :1;

	/*
	 * Number of bytes used for the delta score value:
	 * 0: 0 bytes
	 * 1: 1 byte
	 * 2: 2 bytes
	 * 3: 4 bytes
	 */
	unsigned int deltaScoreSize_ :2;

	/*
	 * Number of bytes used for the first child pointer
	 * 0: 0 bytes
	 * 1: 1 byte
	 * 2: 2 bytes
	 * 3: 4 bytes
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
	 * Returns all characters of this node as a std::string
	 */
	std::string getString() {
		return std::string(getCharacters(), charactersSize_);
	}

	/**
	 * Returns the pointer to the character array
	 */
	u_int32_t getDeltaScore() {
		/*
		 * Cast the storage to an array directly behind the characters and use a bitmap to
		 * only return as many bytes as are used for the delta score
		 */
		return Utils::bytesToUInt32(
				characters_deltaScore_firstChildOffset_ + charactersSize_)
				& bitmaskFor2bValues[deltaScoreSize_];
//		return *(reinterpret_cast<u_int32_t*>(characters_deltaScore_firstChildOffset_
//				+ charactersSize_)) & bitmaskFor2bValues[deltaScoreSize_];
	}

	/**
	 * Returns the pointer to the character array (counted from the first byte of this node)
	 */
	u_int32_t getFirstChildOffset() {
		/*
		 * Cast the storage to an array at the position behind the delta score and use a bitmap to
		 * only return as many bytes as are used for the first child offset
		 */
		return Utils::bytesToUInt32(
				characters_deltaScore_firstChildOffset_ + charactersSize_
						+ numberOfBytesBy2bValue[deltaScoreSize_])
				& bitmaskFor2bValues[firstChildOffsetSize_];
//		return *(reinterpret_cast<u_int32_t*>(characters_deltaScore_firstChildOffset_
//				+ charactersSize_ + numberOfBytesBy2bValue[deltaScoreSize_]))
//				& bitmaskFor2bValues[firstChildOffsetSize_];
	}

	/**
	 * Sets the size and the value of the firstChildOffset
	 * @return Returns how much the length of the node has been increased (may be negative
	 * if length has been decreased)
	 */
	void setFirstChildOffset(const u_int32_t offset) {
		firstChildOffsetSize_ = getNumberOfBytesToStore2b(offset);
		memcpy(
				characters_deltaScore_firstChildOffset_ + charactersSize_
						+ numberOfBytesBy2bValue[deltaScoreSize_], &offset,
				numberOfBytesBy2bValue[firstChildOffsetSize_]);
	}
	u_int32_t getSize() const {
		return sizeof(PackedNode) + charactersSize_
				+ numberOfBytesBy2bValue[deltaScoreSize_]
				+ numberOfBytesBy2bValue[firstChildOffsetSize_];
	}

	bool isLeafNode() {
		return firstChildOffsetSize_ == 0;
	}

	static u_int32_t calculateSize(const u_int8_t characterSize,
			const u_int32_t deltaScore, const u_int32_t firstChildOffset) {
		return sizeof(PackedNode) + characterSize
				+ numberOfBytesBy2bValue[getNumberOfBytesToStore2b(deltaScore)]
				+ numberOfBytesBy2bValue[getNumberOfBytesToStore2b(
						firstChildOffset)];
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
			const char* characters, const bool isLastSibling,
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
	static PackedNode* createNode(char* memory, const char characterSize,
			const char* characters, const bool isLastSibling,
			const u_int32_t deltaScore, const int firstChildOffset);
}__attribute__((packed));

#endif /* PACKEDNODE_H_ */
