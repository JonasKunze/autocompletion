/*
 * CompletionTrieBuilder.h
 *
 *  Created on: Oct 28, 2013
 *      Author: Jonas Kunze
 */

#ifndef COMPLETIONTRIEBUILDER_H_
#define COMPLETIONTRIEBUILDER_H_

#include <sys/types.h>
#include <deque>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "BuilderNode.h"
#include "SuggestionList.h"

class BuilderNode;

class SuggestionStore;

class CompletionTrie;

struct BuilderNodeLayerComparator {
	bool operator()(const BuilderNode* left, const BuilderNode* right);
};

class CompletionTrieBuilder {
public:
	CompletionTrieBuilder();
	virtual ~CompletionTrieBuilder();

	void addString(std::string str, u_int32_t score,
			std::string additionalData);
	CompletionTrie* generateCompletionTrie();
	void print();
	void printNode(BuilderNode* node, std::deque<BuilderNode*> locus);

	float getAverageCharsPerNode() {
		return numberOfCharsStored / (float)allNodes.size();
	}

	size_t getNumberOfNodes() {
		return allNodes.size();
	}

	u_int32_t getNumberOfCharsStored() {
		return numberOfCharsStored;
	}

	u_int32_t getNumberOfWordsStored() {
		return numberOfWordsStored;
	}

	static CompletionTrie* buildFromFile(const std::string fileName);
private:
	BuilderNode* root;
	std::vector<BuilderNode*> allNodes;

	std::shared_ptr<SuggestionStore> suggestionStore;

	// Sum of all word lengths of all terms stored
	u_int32_t numberOfCharsStored;
	u_int32_t numberOfWordsStored;

	BuilderNode* createNode(BuilderNode* parent, u_int32_t score,
			std::string _suffix);

	/*
	 * Reads a file which must consist of lines with following format:
	 * $score\t$term\t$additionalDataMayContainTabs\n
	 *
	 */
	static std::vector<Suggestion> readFile(const std::string fileName);

	/**
	 * Creates a list of nodes defining the longest maximum substr of term. Nodes returned
	 * will never be leaf nodes with only one character
	 *
	 * @param numberOfCharsFound The number of characters that fit term. If it equals term.length() the whole term is
	 * encoded in the returned list of nodes. Else term.substr(numberOfCharsFound) is the remaining suffix that needs
	 * to be stored
	 *
	 * @charsRemainingForLastNode The number of characters of the last node that do not fit term
	 */
	std::stack<BuilderNode*> findLocus(std::string term,
			unsigned short& numberOfCharsFound,
			unsigned char& charsRemainingForLastNode);
	void splitNode(BuilderNode* node, unsigned char splitPos);

};

#endif /* COMPLETIONTRIEBUILDER_H_ */
