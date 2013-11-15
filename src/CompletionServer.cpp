/*
 * CompletionServer.cpp
 *
 *  Created on: Nov 14, 2013
 *      Author: Jonas Kunze
 */

#include "CompletionServer.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <zmq.h>
#include <iosfwd>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <sstream>
#include <thread>
#include <map>

#include "CompletionTrie.h"
#include "SuggestionList.h"
#include "CompletionTrieBuilder.h"

CompletionServer::CompletionServer() :
		builderThread_(&CompletionServer::builderThread, this), trie(nullptr) {
}

CompletionServer::CompletionServer(CompletionTrie* _trie) :
		builderThread_(&CompletionServer::builderThread, this), trie(_trie) {
}

CompletionServer::~CompletionServer() {
}

static std::string formatSuggestion(std::string suggestion, std::string key) {
	std::stringstream ss;
	ss << "{\"suggestion\":\"" << suggestion << "\",\"key\":\"" << key << "\"}";
	return ss.str();
}

std::string CompletionServer::generateResponse(const CompletionTrie* trie,
		char* req, int requestLength) {
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
		jsonStream << formatSuggestion(sugg.suggestion, sugg.URI);

	}
	jsonStream << "]}";

	std::cout << "Generated response: " << jsonStream.str() << std::endl;

	return jsonStream.str();
}

//static int startServer(const CompletionTrie* trie) {
//	/*
//	 * Connect to pull and push socket of sockjsproxy
//	 */
//	zmq::context_t context(1);
//	zmq::socket_t in_socket(context, ZMQ_PULL);
//	in_socket.connect("tcp://localhost:9241");
//
//	zmq::socket_t out_socket(context, ZMQ_PUSH);
//	out_socket.connect("tcp://localhost:9242");
//
//	while (1) {
//		zmq::message_t message_msg;
//		zmq::message_t session_ID_msg;
//		zmq::message_t data;
//
//		in_socket.recv(&message_msg);
//		std::string message(reinterpret_cast<char*>(message_msg.data()),
//				message_msg.size());
//		in_socket.recv(&session_ID_msg);
//
//		uint64_t session_ID = reinterpret_cast<uint64_t>(session_ID_msg.data());
//
//		in_socket.recv(&data);
//
//		if (message == "message") {
//			std::cout << "ID=" << session_ID << std::endl;
//			std::cout << "Message: " << message << std::endl;
//			out_socket.send(message_msg, ZMQ_SNDMORE);
//			out_socket.send(session_ID_msg, ZMQ_SNDMORE);
//
//			long start = Utils::getCurrentMicroSeconds();
//			std::string response = generateResponse(trie,
//					reinterpret_cast<char*>(data.data()), data.size());
//			long time = Utils::getCurrentMicroSeconds() - start;
//			std::cout << "Generating answer took " << time << "µs" << std::endl;
//
//			data.rebuild((unsigned long) response.length());
//			memcpy((void *) data.data(), response.c_str(),
//					(unsigned long) response.length());
//			out_socket.send(data);
//		} else if (strcmp(reinterpret_cast<char*>(message_msg.data()),
//				"connect\0") == 0) {
//			printf("New client: %ld\n", session_ID);
//		} else if (strcmp(reinterpret_cast<char*>(message_msg.data()),
//				"disconnect\0") == 0) {
//			printf("Client disconnected: %ld\n", session_ID);
//		}
//	}
//	return 0;
//}

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

void CompletionServer::builderThread() {
	void *context = zmq_ctx_new();
	void *socket = zmq_socket(context, ZMQ_PULL);
	std::stringstream address;
	address << BUILDER_ZMQ_PROTO << "://*:" << BUILDER_ZMQ_PORT;
	int rc = zmq_bind(socket, address.str().c_str());
	if (rc != 0) {
		std::cerr << "startBuilderThread: Unable to bind to " << address.str()
				<< std::endl;
		exit(1);
	}

	char dataBuffer[1024];
	std::map<uint64_t, CompletionTrieBuilder*> builders;
	builders.clear();

	while (1) {
		uint64_t index;
		while (zmq_recv(socket, &index, sizeof(index), 0) != sizeof(index)) {
			std::cerr
					<< "CompletionServer::builderThread: Unable to receive message"
					<< std::endl;
		}
		uint8_t msg;
		int64_t more;
		zmq_recv(socket, &msg, sizeof(msg), 0);

		if (msg == BUILDER_MSG_INSERT) {
			do {
				/*
				 * Get Term
				 */
				int dataSize = receiveString(socket, sizeof(dataBuffer),
						dataBuffer);
				std::string term(dataBuffer, dataSize);

				/*
				 * Get score
				 */
				uint32_t score;
				zmq_recv(socket, &score, sizeof(score), 0);

				/*
				 * Get URI
				 */
				dataSize = receiveString(socket, sizeof(dataBuffer),
						dataBuffer);
				std::string URI(dataBuffer, dataSize);

				/*
				 * Get Image if there is one more part of the current multi-part message
				 */
				size_t more_size = sizeof more;
				rc = zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &more_size);

				std::string image = "";
				if (rc == 0 && more) {
					dataSize = receiveString(socket, sizeof(dataBuffer),
							dataBuffer);
					image = std::string(dataBuffer, dataSize);
				}
				CompletionTrieBuilder* builder = builders[index];
				if (builder == nullptr) {
					std::cerr
							<< "Trying to add term but no CompletionTrieBuilder exists for index "
							<< index << "!" << std::endl;
				} else {
					std::cout << "Adding Term " << term << "\t" << score << "\t"
							<< image << "\t" << URI << std::endl;
					builder->addString(term, score, image, URI);
				}
				rc = zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &more_size);
			} while (more);

		} else if (msg == BUILDER_MSG_START_BULK) {
			if (builders.count(index)) {
				std::cerr << "Starting trie building of index " << index
						<< " but a triBuilder already exists for this index! Will create a new one."
						<< std::endl;
				delete builders[index];
				builders.erase(index);
			}
			CompletionTrieBuilder* builder = new CompletionTrieBuilder();
			builders.insert(std::make_pair(index, builder));
		} else if (msg == BUILDER_MSG_STOP_BULK) {
			CompletionTrieBuilder* builder = builders[index];
			if (builder == nullptr) {
				std::cerr
						<< "Trying to finish Trie building but no CompletionTrieBuilder exists for index "
						<< index << "!" << std::endl;
			} else {
				builder->print();
				if (this->trie != nullptr) {
					CompletionTrie* tmp = this->trie;
					this->trie = builder->generateCompletionTrie();
					delete tmp;
				} else {
					this->trie = builder->generateCompletionTrie();
				}
				delete builder;
				builders.erase(index);
			}
		}
	}
}

void CompletionServer::start() {
	/*
	 * Connect to pull and push socket of sockjsproxy
	 */
	void *context = zmq_ctx_new();
	void *in_socket = zmq_socket(context, ZMQ_PULL);
	zmq_connect(in_socket, "tcp://localhost:9241");

	void *out_socket = zmq_socket(context, ZMQ_PUSH);
	zmq_connect(out_socket, "tcp://localhost:9242");

	char messageBuffer[13];
	int messageSize;
	char dataBuffer[1500];

	while (1) {
		while ((messageSize = receiveString(in_socket, sizeof(messageBuffer),
				&messageBuffer[0])) <= 0) {
			std::cout << "zmq_recv returned -1" << std::endl;
		}

		uint64_t session_ID;
		zmq_recv(in_socket, &session_ID, sizeof(session_ID), 0);

		int dataSize = receiveString(in_socket, sizeof(dataBuffer), dataBuffer);

		if (strcmp(messageBuffer, "message") == 0) {
			printf("message: %s\n", messageBuffer);
			printf("data: %s\n", dataBuffer);

			zmq_send(out_socket, messageBuffer, messageSize, ZMQ_SNDMORE);
			zmq_send(out_socket, &session_ID, sizeof(session_ID), ZMQ_SNDMORE);

			std::string response = generateResponse(trie, dataBuffer, dataSize);
			zmq_send(out_socket, response.c_str(), response.size(), 0);
		} else if (strcmp(messageBuffer, "connect") == 0) {
			printf("New client: %ld\n", session_ID);
		} else if (strcmp(messageBuffer, "disconnect") == 0) {
			printf("Client disconnected: %ld\n", session_ID);
		}
	}
}
