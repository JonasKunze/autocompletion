/*
 * CompletionTrieBuilder.cpp
 *
 *  Created on: Oct 28, 2013
 *      Author: Jonas Kunze
 */

#include "CompletionTrieBuilder.h"

#include <sys/types.h>
#include <memory>

CompletionTrieBuilder::CompletionTrieBuilder() :
		root(new BuilderNode(0, "")) {

}

CompletionTrieBuilder::~CompletionTrieBuilder() {
	// TODO Auto-generated destructor stub
}

void CompletionTrieBuilder::addString(std::string str, u_int32_t score) {
	if (root->children.size() == 0) {
		std::shared_ptr<BuilderNode> child(new BuilderNode(0xFFFFFFFF, str));
		root->children.insert(child);
	}
}

std::deque<std::shared_ptr<BuilderNode> > CompletionTrieBuilder::findLocus(
		const std::string term, bool& return_foundTerm) {
	std::deque<std::shared_ptr<BuilderNode> > resultLocus;

	return_foundTerm = false;

	std::string remainingPrefix = term;

	std::shared_ptr<BuilderNode> parent = root;

	bool nodeFits = true;

	while (true) {
		for (std::shared_ptr<BuilderNode> node : parent->children) {
			short lastFitPos = -1;
			for (unsigned short i = 0; i < node->suffix.length(); i++) {
				if (remainingPrefix.at(i) != node->suffix.at(i)) {
					break; // for(short i...
				}
				lastFitPos = i;
			}

			/*
			 * If we found a fitting node:
			 */
			if (lastFitPos != -1) {
				resultLocus.push_back(node);

				remainingPrefix = remainingPrefix.substr(lastFitPos);
				if (remainingPrefix.size() == 0) {
					return resultLocus;
				}

				parent = node;
				break; // for (std::shared_ptr...
			}
		}
	}

	return resultLocus;
}
