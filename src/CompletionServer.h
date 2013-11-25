/*
 * CompletionServer.h
 *
 *  Created on: Nov 14, 2013
 *      Author: Jonas Kunze
 */

#ifndef COMPLETIONSERVER_H_
#define COMPLETIONSERVER_H_

#include <thread>
#include <string>

#define BUILDER_ZMQ_PROTO "tcp"
#define BUILDER_ZMQ_PORT "9243"

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

	void start();

private:
	std::thread builderThread_;
	CompletionTrie* trie;
	void builderThread();
};

#endif /* COMPLETIONSERVER_H_ */
