/*
 * CompletionTrieBuilder.cpp
 *
 *  Created on: Oct 28, 2013
 *      Author: Jonas Kunze
 */

#include "CompletionTrieBuilder.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <utility>

#include "../options/Options.h"
#include "../utils/Utils.h"
#include "BuilderNode.h"
#include "CompletionTrie.h"
#include "PackedNode.h"
#include "SuggestionList.h"
#include "SuggestionStore.h"

#define MAXIMUM_PREFIX_SIZE 7

bool BuilderNodeLayerComparator::operator()(const BuilderNode* left,
		const BuilderNode* right) {
//	long l1 = (((long) (left->getTrieLayer() - right->getTrieLayer())) << (32))
//			+ ((long) right->parent->score
//					- left->parent->score);
//	if (l1 == 0) {
//		return left->score > right->score;
//	}
//	return l1 < 0;

	if (left->getTrieLayer() == right->getTrieLayer()) {
		if (left->parent->score_ == right->parent->score_) {
			if (left->parent == right->parent) {
				return left->score_ > right->score_;
			}
			return left->parent < right->parent;
		}
		return left->parent->score_ > right->parent->score_;
	}
	return left->getTrieLayer() < right->getTrieLayer();
}

BuilderNode* CompletionTrieBuilder::createNode(BuilderNode* parent,
		u_int32_t score, std::string suffix) {
	BuilderNode* node = new BuilderNode(parent, score, suffix);
	allNodes.push_back(node);
	return node;
}

CompletionTrieBuilder::CompletionTrieBuilder() :
		suggestionStore(std::make_shared<SuggestionStore>()), numberOfCharsStored(
				0) {
	root = createNode(nullptr, 0xFFFFFFFF, "");
}

CompletionTrieBuilder::~CompletionTrieBuilder() {
	for (BuilderNode* node : allNodes) {
		delete node;
	}
}

std::vector<Suggestion> CompletionTrieBuilder::readFile(
		const std::string fileName) {
	std::vector<Suggestion> nodes;

	std::ifstream myReadFile;
	myReadFile.open(fileName);
	if (myReadFile.fail()) {
		std::cerr << "File does not exist: " << fileName << std::endl;
		exit(1);
	}

	if (myReadFile.is_open()) {
		std::istringstream lin;
		// Read line by line
		for (std::string line; std::getline(myReadFile, line);) {
			lin.clear();
			lin.str(line);
			std::string item;
			// Split line at each \t
			std::vector<std::string> elems;
			while (std::getline(lin, item, '\t')) {
				elems.push_back(item);
			}

			if (elems.size() < 2) {
				std::cerr << "Badly formated line in file: " << line
						<< std::endl;
				exit(1);
			}

			// Concat the last columns
			std::stringstream data;
			for (uint i = 2; i < elems.size(); i++) {
				if (i != 2) {
					data << "\t";
				}
				data << elems[i];
			}
			nodes.push_back( { elems[1], (u_int32_t) std::stoi(elems[0]),
					elems[0] });

		}
	}
	myReadFile.close();
	return nodes;
}

CompletionTrie* CompletionTrieBuilder::buildFromFile(
		const std::string fileName) {
	CompletionTrieBuilder builder;
	long start = Utils::getCurrentMicroSeconds();
	std::vector<Suggestion> nodeValues = readFile(fileName);

	long time = Utils::getCurrentMicroSeconds() - start;
	std::cout << time / 1000. << " ms for reading file" << std::endl;

	start = Utils::getCurrentMicroSeconds();

	/*
	 * Fill the Builder with all terms from the file
	 */
	for (Suggestion nodeValue : nodeValues) {
		builder.addString(nodeValue.suggestion, nodeValue.relativeScore,
				nodeValue.additionalData);
	}
	time = Utils::getCurrentMicroSeconds() - start;
	std::cout << time / 1000. << " ms for creating builder trie" << std::endl;

	std::cout << "Total memory consumption: " << Utils::GetMemUsage() / 1000000.
			<< " MB" << std::endl;

	start = Utils::getCurrentMicroSeconds();

	CompletionTrie* trie = builder.generateCompletionTrie();

	time = Utils::getCurrentMicroSeconds() - start;
	std::cout << time / 1000. << " ms for creating packed trie with "
			<< trie->getMemoryConsumption() << " Bytes" << std::endl;

	std::cout << "Total memory consumption: " << Utils::GetMemUsage() / 1000000.
			<< " MB" << std::endl;

	std::cout << "Number of words stored: " << builder.getNumberOfTerms()
			<< std::endl;
	std::cout << "Average word length: " << builder.getAverageWordLength()
			<< std::endl;
	std::cout << "Average Bytes per word: "
			<< trie->getMemoryConsumption() / builder.getNumberOfTerms()
			<< std::endl;
	return trie;
}

CompletionTrie* CompletionTrieBuilder::generateCompletionTrie() {
	std::sort(allNodes.begin(), allNodes.end(), BuilderNodeLayerComparator());

	const u_int64_t memSize = allNodes.size() * PackedNode::getMaxSize();
	u_int64_t memPointer = memSize;
	char* mem = new char[memPointer + 8];
	/*
	 *  mem is now at last position + 1. Moving N to the left will allow us to write N bytes
	 */

	BuilderNode* lastParent = nullptr;

	for (auto it = allNodes.rbegin(); it != allNodes.rend(); ++it) {
		BuilderNode* node = *it;

		/*
		 * Every time node->trieLayer changes the current node is the last sibling as we are
		 * coming from the right side
		 */
		if (node->parent != lastParent) {
			lastParent = node->parent;
			node->isLastSibbling_ = true;
		}

		u_int32_t nodeSize = node->calculatePackedNodeSize(memPointer);

		/*
		 * Recalculate size as with the new offset the child offset might
		 * need one more byte
		 */
		nodeSize = node->calculatePackedNodeSize(memPointer - nodeSize);

		memPointer -= nodeSize;

		PackedNode* pNode = PackedNode::createNode(mem + memPointer,
				node->suffix_.length(), node->suffix_.c_str(),
				node->isLastSibbling_, node->getDeltaScore(),
				node->firstChildPointer_ == 0 ?
						0 : node->firstChildPointer_ - memPointer);
		suggestionStore->addTerm(pNode, node->additionalData_);

		/*
		 * Update firstChildPointer every time. As we come from the right side the last
		 * update will be the real first child
		 */
		if (node->parent != nullptr) { // Root node does not have a parent
			node->parent->firstChildPointer_ = memPointer;
		}
	}

	char* finalMem = new char[memSize - memPointer];
	memcpy(finalMem, mem + memPointer, memSize - memPointer);

	/*
	 * Tell the suggestion Store where the nodes have been moved by the memcpy
	 */
	suggestionStore->setPointerDelta(
			reinterpret_cast<u_int64_t>(mem) + memPointer
					- reinterpret_cast<u_int64_t>(finalMem));
	delete[] mem;
	return new CompletionTrie(finalMem, memSize - memPointer, suggestionStore);
}

void CompletionTrieBuilder::addString(std::string str, u_int32_t score,
		std::string additionalData) {
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);

	numberOfCharsStored += str.length();

	unsigned short numberOfCharsFound = 0;
	unsigned char charsRemainingForLastNode = 0;
	std::stack<BuilderNode*> locus = findLocus(str, numberOfCharsFound,
			charsRemainingForLastNode);

	BuilderNode* parent = locus.top();

	/*
	 * If the searched term is longer than the string defined by the current parent node
	 */
	if (parent != root && charsRemainingForLastNode > 0
			&& parent->suffix_.length() != 1
			&& charsRemainingForLastNode < parent->suffix_.length()) {

		if (numberOfCharsFound == str.length()
				&& parent->suffix_.length()
						== static_cast<uint>(charsRemainingForLastNode + 1)) {
			/*
			 * E.g. we've added XXXabc and than XXXa. In this case we should not split abc but
			 * add a to abc's parent XXX
			 */
			numberOfCharsFound -= parent->suffix_.length()
					- charsRemainingForLastNode;
			locus.pop();
			parent = locus.top();
		} else {
			/*
			 * Here we found more than one char but not all of parent are identical
			 */
			splitNode(parent,
					parent->suffix_.length() - charsRemainingForLastNode);

		}
	}

	/*
	 * If the parent is already a leaf node
	 */
	if (parent->isLeafNode() && charsRemainingForLastNode == 0
			&& parent != root) {
		if (parent->suffix_.length() != 1) {
			splitNode(parent, parent->suffix_.length() - 1);
			numberOfCharsFound--;
		} else {
			/*
			 * parent is a non splittable leaf node -> take its parent instead
			 */
			numberOfCharsFound -= parent->suffix_.length();
			locus.pop();
			parent = locus.top();
		}
	}

	std::string prefix = str.substr(numberOfCharsFound);

	std::string nodePrefix;
	while ((nodePrefix = prefix.substr(0, MAXIMUM_PREFIX_SIZE)).length() != 0) {
		BuilderNode* child = createNode(parent, score, nodePrefix);
		parent->addChild(child);
		if (prefix.length() > MAXIMUM_PREFIX_SIZE) {
			parent = child;
			prefix = prefix.substr(MAXIMUM_PREFIX_SIZE);
		} else {
			child->setAdditionalData(additionalData);
			break;
		}
	}
}

void CompletionTrieBuilder::splitNode(BuilderNode* node,
		unsigned char splitPos) {
	std::string secondSuffix = node->suffix_.substr(splitPos);
	BuilderNode* secondNode = createNode(node, node->score_, secondSuffix);

	node->suffix_ = node->suffix_.substr(0, splitPos);

	/*
	 * Move children from the original node to the second node
	 */
	secondNode->children = node->children;
	node->children.clear();
	node->addChild(secondNode);
	secondNode->setAdditionalData(node->additionalData_);

	for (BuilderNode* child : secondNode->children) {
		child->setParent(secondNode);
	}
}

std::stack<BuilderNode*> CompletionTrieBuilder::findLocus(
		const std::string term, unsigned short& numberOfCharsFound,
		unsigned char& charsRemainingForLastNode) {
	std::stack<BuilderNode*> resultLocus;

	resultLocus.push(root);

	std::string remainingPrefix = term;

	BuilderNode* parent = root;

	BuilderNode* nextParent = nullptr;
	int nextParentsLastFitPos = -1;

	restart: for (BuilderNode* node : parent->children) {
		short lastFitPos = Utils::findFirstNonMatchingCharacter(
				std::move(node->suffix_.c_str()),
				std::move(remainingPrefix.c_str())) - 1;

		if (lastFitPos != -1 && lastFitPos >= (short) node->suffix_.length()) {
			lastFitPos = node->suffix_.length() - 1;
		} else if (lastFitPos != -1
				&& lastFitPos >= (short) remainingPrefix.length()) {
			lastFitPos = remainingPrefix.length() - 1;
		}

		/*
		 * If we found a fitting node:
		 */
		if (lastFitPos != -1) {
			/*
			 * Ignore leaf nodes with only one character
			 */
			if (node->suffix_.size() == 1 && node->isLeafNode()) {
				if (remainingPrefix.length() == 1) {
					numberOfCharsFound = 1;
					resultLocus.push(node);
					return resultLocus;
				}
				continue;
			}

			nextParent = node;
			nextParentsLastFitPos = lastFitPos;
			break;
		}
	}
	/*
	 * We've gone through all children of parent. Now Let's have a look at the children of the node
	 * with the longest suffix found being `nextParent`
	 */
	if (nextParent != nullptr) {
		charsRemainingForLastNode = nextParent->suffix_.length()
				- nextParentsLastFitPos - 1;
		resultLocus.push(nextParent);

		numberOfCharsFound += nextParentsLastFitPos + 1;

		remainingPrefix = remainingPrefix.substr(nextParentsLastFitPos + 1);
		if (remainingPrefix.size() == 0
				|| static_cast<u_int32_t>(nextParentsLastFitPos + 1)
						!= nextParent->suffix_.length()) {
			return resultLocus;
		}
		parent = nextParent;
		nextParent = nullptr;
		goto restart;
	}

	return resultLocus;
}

void CompletionTrieBuilder::print() {
	if (!Options::VERBOSE) {
		return;
	}
	std::cout << "============ CompletionTrieBuilder ============" << std::endl;

	std::deque<BuilderNode*> locus;
	std::cout << "graph completionTrie {" << std::endl;

	locus.push_back(root);
	printNode(root, locus);

	std::cout << "}" << std::endl;

	std::cout << "================" << std::endl;
	for (BuilderNode* node : allNodes) {
		std::cout << node->suffix_ << "\t" << node->trieLayer << "\t"
				<< node->isLastSibbling_ << "\t" << node->score_ << std::endl;
	}
	std::cout << "========CompletionTrieBuilder::print END========"
			<< std::endl;
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

		if (parent->suffix_.length() == 0) {
			/*
			 * the root element
			 */
			std::cout << "ROOT";
		} else {
			std::cout << parent->suffix_;
		}
		std::cout << " -- " << child->suffix_ << " : " << child->score_
				<< std::endl;
	}
}
