//============================================================================
// Name        : main.cpp
// Author      : Jonas Kunze
// Version     :
// Copyright   : GPLv3
// Description : Main method of the autocompletion service
//============================================================================

//#include <cstring>

//#include <deque>

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <zmq.h>
#include <sstream>

#include "CompletionTrie.h"
#include "CompletionTrieBuilder.h"
#include "SuggestionList.h"
#include "PerformanceTest.h"

using namespace std;

static int receiveString(void *socket, const unsigned short length,
		char* buffer) {
	int size = zmq_recv(socket, buffer, length, 0);
	if (size == -1)
		return size;
	if (size > length)
		size = length;

	buffer[size] = '\0';
	return size;
}

std::string formatSuggestion(std::string suggestion, std::string key) {
	std::stringstream ss;
	ss << "{\"suggestion\":\"" << suggestion << "\",\"key\":\"" << key << "\"}";
	return ss.str();
}

static std::string generateResponse(const CompletionTrie* trie, char* req,
		int requestLength) {
	std::string request(req, requestLength);

	std::shared_ptr<SuggestionList> suggestions = trie->getSuggestions(request,
			10);

	std::stringstream jsonStream;
	jsonStream << "{\"suggestionList\": [";
	bool isFirstSuggestion = true;
	for (Suggestion sugg : suggestions->suggestedWords) {
		if (!isFirstSuggestion) {
			jsonStream << ",";
		} else {
			isFirstSuggestion = false;
		}
		jsonStream << formatSuggestion(sugg.suggestion, sugg.URL);

	}
	jsonStream << "]}";

//			"{\"suggestionList\": [{\"key\": 1, \"suggestion\": \"Anna\"}, {\"key\": 2, \"suggestion\": \"Berta\"}, {\"key\": 3, \"suggestion\": \"Carolin\"}, {\"key\": 4, \"suggestion\": \"Dorothee\"}, {\"key\": \"\", \"suggestion\": \"df\"}]}"
	return jsonStream.str();
}

static int startServer(const CompletionTrie* trie) {
	/*
	 * Connect to pull and push socket of sockjsproxy
	 */
	void *context = zmq_ctx_new();
	void *in_socket = zmq_socket(context, ZMQ_PULL);
	zmq_connect(in_socket, "tcp://localhost:9241");

	void *out_socket = zmq_socket(context, ZMQ_PUSH);
	int status = zmq_connect(out_socket, "tcp://localhost:9242");
	std::cout << status << "!!!" << std::endl;

	char messageBuffer[13];
	char dataBuffer[1500];
	while (1) {
		int messageSize = receiveString(in_socket, sizeof(messageBuffer),
				&messageBuffer[0]);
		uint64_t session_ID;
		zmq_recv(in_socket, &session_ID, sizeof(session_ID), 0);

		receiveString(in_socket, sizeof(dataBuffer), dataBuffer);

		if (strcmp(messageBuffer, "message\0") == 0) {
			printf("message: %s\n", messageBuffer);
			printf("data: %s\n", dataBuffer);
			zmq_send(out_socket, messageBuffer, messageSize, ZMQ_SNDMORE);
			zmq_send(out_socket, &session_ID, sizeof(session_ID), ZMQ_SNDMORE);

			std::string response = generateResponse(trie, dataBuffer,
					messageSize);
			zmq_send(out_socket, response.c_str(), response.size(), 0);
		} else if (strcmp(messageBuffer, "connect\0") == 0) {
			printf("New client: %ld\n", session_ID);
		} else if (strcmp(messageBuffer, "disconnect\0") == 0) {
			printf("Client disconnected: %ld\n", session_ID);
		}
	}
	return 0;
}

int main() {
//	CompletionTrieBuilder builder;
//	builder.addString("a", 15078, std::string("image"), std::string("url"));
//	builder.addString("c", 13132, "image", "url");
//	builder.addString("b", 13132, "image", "url");
//
//////
//////	builder.addString("'Outstanding", 175);
//////	builder.addString("'Operation", 141);
//////	builder.addString("'Open", 92);
//////	builder.addString("'", 92);
//////	builder.print();
//////
//////	builder.addString("abcdefg", 1235);
//////	builder.addString("a1234567b	", 1236);
//////	builder.addString("ab", 1236);
//////
//////	builder.addString("abcdefgh", 1235);
//////	builder.addString("abcdefgh", 1236);
//////
//////	builder.addString("abc", 1235);
//////	builder.addString("abc", 1236);
//////
//////	builder.addString("a", 1235);
//////	builder.addString("a", 1236);
//////
//////	builder.addString("abcd", 1235);
//////	builder.addString("a", 1236);
//////	builder.addString("ae", 1236);
//////	builder.addString("afcd", 1235);
//////
//////	builder.addString("a", 1235);
//////	builder.addString("abc", 1236);
//////	builder.addString("ade", 1236);
//////
//////	builder.addString("abc", 1236);
//////	builder.addString("abe", 1235);
//////	builder.addString("ade", 1236);
//
//	const CompletionTrie* trie = builder.generateCompletionTrie();
//	builder.print();
//
//	trie->print();
//	std::shared_ptr<SuggestionList> suggestions = trie->getSuggestions("a", 10);
//
//	for (Suggestion sugg : suggestions->suggestedWords) {
//		std::cout << sugg.suggestion << "\t" << sugg.relativeScore << "\t"
//				<< sugg.URL << "\t" << sugg.image << std::endl;
//	}

	CompletionTrie* trie = performanceTest();
	startServer(trie);

	return 0;
}
