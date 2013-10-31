/*
 * CompletionTrieBuilder.cpp
 *
 *  Created on: Oct 28, 2013
 *      Author: Jonas Kunze
 */

#include "CompletionTrieBuilder.h"

#include <sys/types.h>
#include <iostream>
#include <memory>

#define MAXIMUM_PREFIX_SIZE 7

CompletionTrieBuilder::CompletionTrieBuilder() :
		root(new BuilderNode(std::shared_ptr<BuilderNode>(NULL), 0, "")) {

}

CompletionTrieBuilder::~CompletionTrieBuilder() {
	// TODO Auto-generated destructor stub
}

void CompletionTrieBuilder::addString(std::string str, u_int32_t score) {
	if (root->children.size() == 0) {
		std::shared_ptr<BuilderNode> child(
				new BuilderNode(root, 0xFFFFFFFF, str));
		root->children.insert(child);
	} else {
		u_int32_t parentScore = 0;

		unsigned short numberOfCharsFound = 0;
		unsigned char charsRemainingForLastNode = 0;
		std::deque<std::shared_ptr<BuilderNode> > locus = findLocus(str,
				numberOfCharsFound, parentScore, charsRemainingForLastNode);

		std::shared_ptr<BuilderNode> parent = locus.back();

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
					std::shared_ptr<BuilderNode>(
							new BuilderNode(parent, score - parentScore,
									nodePrefix)));

			if (prefix.length() >= MAXIMUM_PREFIX_SIZE) {
				prefix = prefix.substr(MAXIMUM_PREFIX_SIZE);
			} else {
				break;
			}
		}

	}
}

void CompletionTrieBuilder::splitNode(std::shared_ptr<BuilderNode> node,
		unsigned char splitPos) {
	std::string secondSuffix = node->suffix.substr(splitPos);
	std::shared_ptr<BuilderNode> secondNode = std::shared_ptr<BuilderNode>(
			new BuilderNode(node, node->deltaScore, secondSuffix));

	node->suffix = node->suffix.substr(0, splitPos);
	node->addChild(secondNode);
}

std::deque<std::shared_ptr<BuilderNode> > CompletionTrieBuilder::findLocus(
		const std::string term, unsigned short& numberOfCharsFound,
		u_int32_t& score, unsigned char& charsRemainingForLastNode) {
	score = 0xFFFFFFFF;
	std::deque<std::shared_ptr<BuilderNode> > resultLocus;

	std::string remainingPrefix = term;

	std::shared_ptr<BuilderNode> parent = root;

	restart: for (std::shared_ptr<BuilderNode> node : parent->children) {
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
	std::deque<std::shared_ptr<BuilderNode> > locus;
	std::cout << "graph completionTrie {" << std::endl;

	locus.push_back(root);
	printNode(root, locus);

	std::cout << "}" << std::endl;

	std::cout << "================" << std::endl;
	for (std::shared_ptr<BuilderNode> node : BuilderNode::allNodes) {
		std::cout << node.get() << "!!" << std::endl;
		std::cout << node.get()->suffix.size() << "!!" << std::endl;
	}
}

/**
 * Recursively prints a node and all its children in the dot format "parent -- child"
 */
void CompletionTrieBuilder::printNode(std::shared_ptr<BuilderNode> parent,
		std::deque<std::shared_ptr<BuilderNode> > locus) {

	for (std::shared_ptr<BuilderNode> child : parent->children) {
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
