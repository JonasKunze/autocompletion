/*
 * CompletionTrieBuilder.cpp
 *
 *  Created on: Oct 28, 2013
 *      Author: Jonas Kunze
 */

#include "CompletionTrieBuilder.h"

#include <sys/types.h>
#include <iostream>
#include <iterator>

#include "CompletionTrie.h"
#include "PackedNode.h"

#define MAXIMUM_PREFIX_SIZE 7

CompletionTrieBuilder::CompletionTrieBuilder() :
		root(new BuilderNode(NULL, 0, "")) {
}

CompletionTrieBuilder::~CompletionTrieBuilder() {
	// TODO Auto-generated destructor stub
}

CompletionTrie* CompletionTrieBuilder::generateCompletionTrie() {
	const u_int32_t memSize = BuilderNode::allNodes.size()
			* PackedNode::getMaxSize();
	u_int32_t memPointer = memSize;
	char* mem = new char[memPointer];
	/*
	 *  mem is now at last position + 1. Moving N to the left will allow us to write N bytes
	 */

	u_int16_t currentLayer = 0;

	for (auto it = BuilderNode::allNodes.rbegin();
			it != BuilderNode::allNodes.rend(); ++it) {
		BuilderNode* node = *it;
		/*
		 * Every time node->trieLayer changes the current node is the last sibling as we are
		 * coming from the right side
		 */
		if (node->trieLayer != currentLayer) {
			currentLayer = node->trieLayer;
			node->isLastSibbling = true;
		}
		/*
		 * The root node has no deltaScore as we'll hardcode the 0xffffffff
		 */
		if (node->trieLayer == 0) {
			node->deltaScore = 0;
		}
//		std::cout << node->suffix << " : " << (int) node->trieLayer
//				<< std::endl;

		memPointer -= node->calculatePackedNodeSize();
		PackedNode* pNode = PackedNode::createNode(mem + memPointer,
				node->suffix.length(), node->suffix.c_str(),
				node->isLastSibbling, node->deltaScore,
				node->firstChildPointer - memPointer);
		std::cout << memPointer << " ! " << (int) pNode->getSize() << " : "
				<< (int) node->calculatePackedNodeSize() << std::endl;
		/*
		 * Update firstChildPointer every time. As we come from the right side the last
		 * update will be the real first child
		 */
		if (node->parent != NULL) { // Root node does not have a parent
			node->parent->firstChildPointer = memPointer;
		}
	}

	char* finalMem = new char[memSize - memPointer];
	memcpy(finalMem, mem + memPointer, memSize - memPointer);

	return new CompletionTrie(mem, memSize);
}

void CompletionTrieBuilder::addString(std::string str, u_int32_t score) {
	if (root->children.size() == 0) {
		BuilderNode* child(new BuilderNode(root, 0xFFFFFFFF, str));
		root->children.insert(child);
	} else {
		u_int32_t parentScore = 0;

		unsigned short numberOfCharsFound = 0;
		unsigned char charsRemainingForLastNode = 0;
		std::deque<BuilderNode*> locus = findLocus(str, numberOfCharsFound,
				parentScore, charsRemainingForLastNode);

		BuilderNode* parent = locus.back();

		if (numberOfCharsFound == str.length()) {
			// the whole term was found
			std::cout << "Trying to add '" << str << "' for the second time"
					<< std::endl;
			return;
		}

		if (charsRemainingForLastNode != 0) {
			splitNode(parent,
					parent->suffix.length() - charsRemainingForLastNode);
		}

		std::string prefix = str.substr(numberOfCharsFound);

		std::string nodePrefix;
		while ((nodePrefix = prefix.substr(0, MAXIMUM_PREFIX_SIZE)).length()
				!= 0) {
			parent->addChild(

			new BuilderNode(parent, score - parentScore, nodePrefix));

			if (prefix.length() >= MAXIMUM_PREFIX_SIZE) {
				prefix = prefix.substr(MAXIMUM_PREFIX_SIZE);
			} else {
				break;
			}
		}

	}
}

void CompletionTrieBuilder::splitNode(BuilderNode* node,
		unsigned char splitPos) {
	std::string secondSuffix = node->suffix.substr(splitPos);
	BuilderNode* secondNode = new BuilderNode(node, node->deltaScore,
			secondSuffix);

	node->suffix = node->suffix.substr(0, splitPos);
	node->addChild(secondNode);
}

std::deque<BuilderNode*> CompletionTrieBuilder::findLocus(
		const std::string term, unsigned short& numberOfCharsFound,
		u_int32_t& score, unsigned char& charsRemainingForLastNode) {
	score = 0xFFFFFFFF;
	std::deque<BuilderNode*> resultLocus;

	std::string remainingPrefix = term;

	BuilderNode* parent = root;

	restart: for (BuilderNode* node : parent->children) {
		short lastFitPos = -1;
		for (unsigned short i = 0; i < node->suffix.length(); i++) {
			if (remainingPrefix.at(i) != node->suffix.at(i)) {
				break; // for(short i...
			}
			lastFitPos = i;
			++numberOfCharsFound;
		}
		charsRemainingForLastNode = node->suffix.length() - lastFitPos - 1;

		/*
		 * If we found a fitting node:
		 */
		if (lastFitPos != -1) {
			resultLocus.push_back(node);
			score -= node->deltaScore;

			remainingPrefix = remainingPrefix.substr(lastFitPos + 1);
			if (remainingPrefix.size() == 0) {
				return resultLocus;
			}

			parent = node;
			goto restart;
		}
	}

	return resultLocus;
}

void CompletionTrieBuilder::print() {
	std::deque<BuilderNode*> locus;
	std::cout << "graph completionTrie {" << std::endl;

	locus.push_back(root);
	printNode(root, locus);

	std::cout << "}" << std::endl;

	std::cout << "================" << std::endl;
	for (BuilderNode* node : BuilderNode::allNodes) {
		std::cout << node->suffix << "!!" << std::endl;
	}
}

/**
 * Recursively prints a node and all its children in the dot format "parent -- child"
 */
void CompletionTrieBuilder::printNode(BuilderNode* parent,
		std::deque<BuilderNode*> locus) {

	for (BuilderNode* child : parent->children) {
		locus.push_back(child);
		if (child->children.size() != 0) {
			printNode(child, locus);
		}
		locus.pop_back();

		if (parent->suffix.length() == 0) {
			/*
			 * the root element
			 */
			std::cout << "ROOT";
		} else {
			std::cout << parent->suffix;
		}
		std::cout << " -- " << child->suffix << std::endl;
	}
}
