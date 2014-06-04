/*
 * CompletionServer.h
 *
 *  Created on: Nov 14, 2013
 *      Author: Jonas Kunze
 *      This server binds to a zmq socket which can be used to import term. It also connects to
 *      a sockjsproxy server which is the gateway to the clients for autocompletion requests.
 *
 *
 *      Adding terms works the following way. You first have to create a new index with a
 *      multimessage including two messages:
 *      1. message: $index (8 byte)
 *      2. message: "startBulk"
 *
 *      Then adding terms works with any number of multimessages, each consists of 5+3*N messages:
 *      1. message: $index (8 byte)
 *      2. message: "addTerm"
 *      do{
 *      3. message: $term
 *      4. message: $score
 *      5. message: $additionalData
 *      while(moreTermsToAdd);
 *
 *      Suggesiont requests to the newly created index will only be replied after you've finished the bulk import:
 *      1. message: $index (8 byte)
 *      2. message: "stopBulk"
 *
 *      The suggestion requests are sent via sockjs to sockjsproxy which will add a session ID to any request. So
 *      only at the first request you have to tell which index you want to send your requests to and then any
 *      following request only needs to consist of the acutal string to be completed:
 *      1. message: 8 byte long suggestion index ID
 *      nth message: Any string to be completed
 */

#ifndef COMPLETIONSERVER_H_
#define COMPLETIONSERVER_H_

#include <thread>
#include <string>
#include <map>

#define BUILDER_MSG_START_BULK "startBulk"
#define BUILDER_MSG_INSERT  "addTerm"
#define BUILDER_MSG_STOP_BULK  "stopBulk"

class CompletionTrie;

class CompletionServer {
public:
	CompletionServer();
	CompletionServer(CompletionTrie* trie);
	virtual ~CompletionServer();

	std::string generateResponse(const CompletionTrie* trie, char* req,
			int requestLength);

	void run();

private:
	std::thread builderThread_;
	std::map<uint64_t, CompletionTrie*> trieByIndex;

	std::map<uint64_t, uint64_t> indexBySession;

	void builderThread();
};

#endif /* COMPLETIONSERVER_H_ */
