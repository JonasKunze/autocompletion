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
#include <zmq.hpp>
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
//	void *context = zmq_ctx_new();
//	void *in_socket = zmq_socket(context, ZMQ_PULL);
//	zmq_connect(in_socket, "tcp://localhost:9241");
//
//	void *out_socket = zmq_socket(context, ZMQ_PUSH);
//	int status = zmq_connect(out_socket, "tcp://localhost:9242");
//	std::cout << status << "!!!" << std::endl;
//  Prepare our context and socket
	zmq::context_t context(1);
	zmq::socket_t in_socket(context, ZMQ_PULL);
	in_socket.connect("tcp://localhost:9241");

	zmq::socket_t out_socket(context, ZMQ_PUSH);
	out_socket.connect("tcp://localhost:9242");

	while (1) {
		zmq::message_t message_msg;
		zmq::message_t session_ID_msg;
		zmq::message_t data;

		in_socket.recv(&message_msg);
		std::string message(reinterpret_cast<char*>(message_msg.data()),
				message_msg.size());
		in_socket.recv(&session_ID_msg);

		uint64_t session_ID = reinterpret_cast<uint64_t>(session_ID_msg.data());

		in_socket.recv(&data);

		if (message == "message") {
			std::cout << "ID=" << session_ID << std::endl;
			std::cout << "Message: " << message << std::endl;
			out_socket.send(message_msg, ZMQ_SNDMORE);
			out_socket.send(session_ID_msg, ZMQ_SNDMORE);

			long start = Utils::getCurrentMicroSeconds();
			std::string response = generateResponse(trie,
					reinterpret_cast<char*>(data.data()), data.size());
			long time = Utils::getCurrentMicroSeconds() - start;
			std::cout << "Generating answer took " << time << "µs" << std::endl;

			data.rebuild((unsigned long) response.length());
			memcpy((void *) data.data(), response.c_str(),
					(unsigned long) response.length());
			out_socket.send(data);
		} else if (strcmp(reinterpret_cast<char*>(message_msg.data()),
				"connect\0") == 0) {
			printf("New client: %ld\n", session_ID);
		} else if (strcmp(reinterpret_cast<char*>(message_msg.data()),
				"disconnect\0") == 0) {
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
