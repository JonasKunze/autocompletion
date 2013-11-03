/*
 * CompletionTrieBuilder.cpp
 *
 *  Created on: Oct 28, 2013
 *      Author: Jonas Kunze
 */

#include "CompletionTrieBuilder.h"

#include <sys/types.h>
#include <algorithm>
#include <cstring>
#include <deque>
#include <iostream>
#include <iterator>
#include <vector>

#include "CompletionTrie.h"
#include "PackedNode.h"

#define MAXIMUM_PREFIX_SIZE 7

CompletionTrieBuilder::CompletionTrieBuilder() :
		root(new BuilderNode(NULL, 0xFFFFFFFF, "")) {
}

CompletionTrieBuilder::~CompletionTrieBuilder() {
	for (BuilderNode* node : BuilderNode::allNodes) {
		delete node;
	}
}

CompletionTrie* CompletionTrieBuilder::generateCompletionTrie() {
	std::sort(BuilderNode::allNodes.begin(), BuilderNode::allNodes.end(),
			BuilderNodeLayerComparator());

	const u_int32_t memSize = BuilderNode::allNodes.size()
			* PackedNode::getMaxSize();
	u_int32_t memPointer = memSize;
	char* mem = new char[memPointer];
	/*
	 *  mem is now at last position + 1. Moving N to the left will allow us to write N bytes
	 */

	BuilderNode* lastParent = NULL;

	for (auto it = BuilderNode::allNodes.rbegin();
			it != BuilderNode::allNodes.rend(); ++it) {
		BuilderNode* node = *it;
		/*
		 * Every time node->trieLayer changes the current node is the last sibling as we are
		 * coming from the right side
		 */
		if (node->parent != lastParent) {
			lastParent = node->parent;
			node->isLastSibbling = true;
		}
		/*
		 * The root node has no deltaScore as we'll hardcode the 0xffffffff
		 */
		if (node->trieLayer == 0) {
			node->score = 0;
		}

		u_int32_t nodeSize = node->calculatePackedNodeSize(memPointer);
		/*
		 * Recalculate size as with the new offset the child offset might
		 * need one more byte
		 */
		nodeSize = node->calculatePackedNodeSize(memPointer - nodeSize);

		memPointer -= nodeSize;

		PackedNode::createNode(mem + memPointer, node->suffix.length(),
				node->suffix.c_str(), node->isLastSibbling,
				node->getDeltaScore(),
				node->firstChildPointer == 0 ?
						0 : node->firstChildPointer - memPointer);

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
	delete[] mem;
	return new CompletionTrie(finalMem, memSize - memPointer);
}

void CompletionTrieBuilder::addString(std::string str, u_int32_t score) {
	unsigned short numberOfCharsFound = 0;
	unsigned char charsRemainingForLastNode = 0;

	std::stack<BuilderNode*> locus = findLocus(str, numberOfCharsFound,
			charsRemainingForLastNode);

	BuilderNode* parent = locus.top();

	if (numberOfCharsFound == str.length() && charsRemainingForLastNode == 0
			&& parent->isLeafNode()) {
		// the whole term was found
		std::cout << "Trying to add '" << str << "' for the second time"
				<< std::endl;
		return;
	}

	if (parent != root && charsRemainingForLastNode != 0
			&& charsRemainingForLastNode < parent->suffix.length() - 1) {
		/*
		 * E.g. we've added abc and than ad. We need to split abc ad position 1
		 * so that we have a->bc and cann add ad to a (than we have a->d, a->bc).
		 *
		 * In this case charsRemainingForLastNode will be 1
		 */
		splitNode(parent,
				parent->suffix.length() - charsRemainingForLastNode - 1);
		numberOfCharsFound--;
	} else if (parent != root
			&& charsRemainingForLastNode == parent->suffix.length() - 1) {
		/*
		 * E.g. we've added abc and than add a. We do not need to split abc as
		 * we need a separate node 'a' as a leaf node. The new node's parent will be
		 * the parent of abc in this case
		 */
		numberOfCharsFound -= parent->suffix.length()
				- charsRemainingForLastNode;
		locus.pop();
		parent = locus.top();
	}

	/*
	 * If the parent is already a leaf node
	 */
	if (parent->isLeafNode() && charsRemainingForLastNode == 0
			&& parent != root) {
		if (parent->suffix.length() != 1) {
			splitNode(parent, parent->suffix.length() - 1);
			numberOfCharsFound--;
		} else {
			numberOfCharsFound -= parent->suffix.length();
			locus.pop();
			parent = locus.top();
		}
	}

	std::string prefix = str.substr(numberOfCharsFound);

	std::string nodePrefix;
	while ((nodePrefix = prefix.substr(0, MAXIMUM_PREFIX_SIZE)).length() != 0) {

		if (parent->score < score) {
			parent->score = score;
		}
		parent->addChild(new BuilderNode(parent, score, nodePrefix));

		if (prefix.length() >= MAXIMUM_PREFIX_SIZE) {
			prefix = prefix.substr(MAXIMUM_PREFIX_SIZE);
		} else {
			break;
		}
	}
}

void CompletionTrieBuilder::splitNode(BuilderNode* node,
		unsigned char splitPos) {
	std::string secondSuffix = node->suffix.substr(splitPos);
	BuilderNode* secondNode = new BuilderNode(node, node->score, secondSuffix);

	node->suffix = node->suffix.substr(0, splitPos);

	/*
	 * Move children from the original node to the second node
	 */
	secondNode->children = node->children;
	node->children.clear();
	node->addChild(secondNode);
}

std::stack<BuilderNode*> CompletionTrieBuilder::findLocus(
		const std::string term, unsigned short& numberOfCharsFound,
		unsigned char& charsRemainingForLastNode) {
	std::stack<BuilderNode*> resultLocus;

	resultLocus.push(root);

	std::string remainingPrefix = term;

	BuilderNode* parent = root;

	BuilderNode* nextParent = NULL;
	short nextParentsLastFitPos = -1;
	short nextParentsNumberOfCharsFound = 0;
	restart: for (BuilderNode* node : parent->children) {
		/*
		 * Ignore leaf nodes with only one character
		 */
		if (node->suffix.size() == 1 && node->isLeafNode()) {
			continue;
		}

		short lastFitPos = -1;
		short numberOfCharsFoundCurrent = 0;
		for (unsigned short i = 0;
				i < node->suffix.length() && i < remainingPrefix.length();
				i++) {
			if (remainingPrefix.at(i) != node->suffix.at(i)) {
				break; // for(short i... // Character at i does not fit
			}
			lastFitPos = i;
			++numberOfCharsFoundCurrent;
		}

		/*
		 * If we found a fitting node:
		 */
		if (lastFitPos != -1) {
			/*
			 * If the node has a longer suffix than nextParent, nextParent will be set to node
			 */
			if (nextParent == NULL
					|| node->suffix.length() > nextParent->suffix.length()) {
				nextParent = node;
				nextParentsLastFitPos = lastFitPos;
				nextParentsNumberOfCharsFound = numberOfCharsFoundCurrent;
			}
		}
	}
	/*
	 * We've gone through all children of parent. Now Let's have a look at the children of the node
	 * with the longest suffix found being `nextParent`
	 */
	if (nextParent != NULL) {
		charsRemainingForLastNode = nextParent->suffix.length()
				- nextParentsLastFitPos - 1;
		resultLocus.push(nextParent);

		numberOfCharsFound += nextParentsNumberOfCharsFound;

		remainingPrefix = remainingPrefix.substr(nextParentsLastFitPos + 1);
		if (remainingPrefix.size() == 0) {
			return resultLocus;
		}

		parent = nextParent;
		nextParent = NULL;
		goto restart;
	}

	return resultLocus;
}

void CompletionTrieBuilder::print() {
	std::deque<BuilderNode*> locus;
	std::cout << "graph completionTrie {" << std::endl;

	locus.push_back(root);
	printNode(root, locus);

	std::cout << "}" << std::endl;

//	std::cout << "================" << std::endl;
//	for (BuilderNode* node : BuilderNode::allNodes) {
//		std::cout << node->suffix << "\t" << node->trieLayer << "\t"
//				<< node->isLastSibbling << "!!" << node->children.size()
//				<< std::endl;
//	}
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
		std::cout << " -- " << child->suffix /*<< " : " << child->isLastSibbling*/
		<< std::endl;
	}
}
