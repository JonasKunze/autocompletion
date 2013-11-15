//============================================================================
// Name        : main.cpp
// Author      : Jonas Kunze
// Version     :
// Copyright   : GPLv3
// Description : Main method of the autocompletion service
//============================================================================

//#include <cstring>

//#include <deque>

#include "CompletionServer.h"
#include "CompletionTrieBuilder.h"
#include "options/Options.h"
#include "PerformanceTest.h"

using namespace std;

int main(int argc, char* argv[]) {
	Options::Initialize(argc, argv);

	CompletionTrie* trie = CompletionTrieBuilder::buildFromFile(
			Options::GetString(OPTION_LOAD_FILE));

	PerformanceTest::runTest(trie);

	//	CompletionServer server(trie);
//	server.start();

	return 0;
}
