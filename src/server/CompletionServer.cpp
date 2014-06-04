/*
 * CompletionServer.cpp
 *
 *  Created on: Nov 14, 2013
 *      Author: Jonas Kunze
 */

#include "CompletionServer.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <zmq.h>
#include <algorithm>
#include <cstdlib>
#include <iosfwd>
#include <iostream>
#include <map>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <thread>

#include "../storage/CompletionTrie.h"
#include "../storage/CompletionTrieBuilder.h"
#include "../storage/SuggestionList.h"
#include "../options/Options.h"

CompletionServer::CompletionServer() :
		builderThread_(&CompletionServer::builderThread, this) {
}

CompletionServer::~CompletionServer() {
}

static std::string formatSuggestion(const Suggestion sugg) {
	std::stringstream ss;
	ss << "{\"suggestion\":\"" << sugg.suggestion << ",\"data\":\""
			<< sugg.additionalData << "\"}";
	return ss.str();
}

std::string CompletionServer::generateResponse(const CompletionTrie* trie,
		char* req, int requestLength) {
	if (trie == nullptr) {
		return "";
	}
	std::string request(req, requestLength);

	std::shared_ptr<SuggestionList> suggestions = trie->getSuggestions(request,
			10);

	std::stringstream jsonStream;
	jsonStream << "{\"suggestionList\": [";
	bool isFirstSuggestion = true;
	for (const Suggestion sugg : suggestions->suggestedWords) {
		if (!isFirstSuggestion) {
			jsonStream << ",";
		} else {
			isFirstSuggestion = false;
		}
		jsonStream << formatSuggestion(sugg);
	}
	jsonStream << "]}";

	std::cout << "Generated response: " << jsonStream.str() << std::endl;

	return jsonStream.str();
}

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

static std::string receiveString(void *socket) {
	zmq_msg_t msg;
	zmq_msg_init(&msg);
	int dataSize = zmq_recvmsg(socket, &msg, 0);
	return std::string(std::move(std::move((char*) zmq_msg_data(&msg))),
			dataSize);
}

void CompletionServer::builderThread() {
	void *context = zmq_ctx_new();
	void *socket = zmq_socket(context, ZMQ_PULL);
	int rc = zmq_bind(socket,
			Options::GetString(OPTION_ZMQ_LISTEN_ADDRESS).c_str());
	if (rc != 0) {
		std::cerr << "startBuilderThread: Unable to bind to "
				<< Options::GetString(OPTION_ZMQ_LISTEN_ADDRESS) << std::endl;
		exit(1);
	}

	std::map<uint64_t, CompletionTrieBuilder*> builders;
	builders.clear();

	while (1) {
		/*
		 * First message: Index
		 */
		uint64_t index;
		while (zmq_recv(socket, &index, sizeof(index), 0) != sizeof(index)) {
			std::cerr
					<< "CompletionServer::builderThread: Unable to receive message"
					<< std::endl;
		}

		/*
		 * Second message: Message type
		 */
		std::string msgType = receiveString(socket);
		int64_t more;

		if (msgType == BUILDER_MSG_INSERT) {
			do {
				/*
				 * 3rd message: Term
				 */
				std::string term = receiveString(socket);

				/*
				 * 4th message: Score
				 */
				uint32_t score;
				zmq_recv(socket, &score, sizeof(score), 0);

				/*
				 * 5th message: additional data to be stored
				 */
				std::string additionalData = receiveString(socket);

				CompletionTrieBuilder* builder = builders[index];

				if (builder == nullptr) {
					std::cerr
							<< "Trying to add term but no CompletionTrieBuilder exists for index "
							<< index << "!" << std::endl;
				} else {
					std::cout << "Adding Term " << term << "\t" << score << "\t"
							<< additionalData << std::endl;
					builder->addString(term, score, additionalData);
				}
				size_t more_size = sizeof more;
				rc = zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &more_size);
			} while (more);
		} else if (msgType == BUILDER_MSG_START_BULK) {
			std::cout << "Received Start Bulk command for index " << index
					<< std::endl;

			if (builders.find(index) != builders.end()) {
				std::cerr << "Starting trie building of index " << index
						<< " but a triBuilder already exists for this index! Will create a new one."
						<< std::endl;
				delete builders[index];
				builders.erase(index);
			}
			CompletionTrieBuilder* builder = new CompletionTrieBuilder();
			builders[index] = builder;
			builder->print();
		} else if (msgType == BUILDER_MSG_STOP_BULK) {
			std::cout << "Received Stop Bulk command for index " << index
					<< std::endl;
			CompletionTrieBuilder* builder = builders[index];
			if (builder == nullptr) {
				std::cerr
						<< "Trying to finish Trie building but no CompletionTrieBuilder exists for index "
						<< index << "!" << std::endl;
			} else {
				builder->print();

				std::map<uint64_t, CompletionTrie*>::iterator lb =
						trieByIndex.lower_bound(index);
				if (lb != trieByIndex.end()
						&& !(trieByIndex.key_comp()(index, lb->first))) {
					/*
					 * A trie with this index already exists -> overwrite it
					 */
					CompletionTrie* tmp = (*lb).second;
					trieByIndex.insert(lb,
							std::map<uint64_t, CompletionTrie*>::value_type(
									index, builder->generateCompletionTrie()));
					delete tmp;
				} else {
					/*
					 * The trie with this index didn't exist yet -> create it
					 */
					trieByIndex.insert(lb,
							std::map<uint64_t, CompletionTrie*>::value_type(
									index, builder->generateCompletionTrie()));
				}

				delete builder;
				builders.erase(index);
			}
		}
	}
}

void CompletionServer::run() {
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
			printf("data: %s\n", dataBuffer);

			/*
			 * Headers for the proxy
			 */
			zmq_send(out_socket, messageBuffer, messageSize, ZMQ_SNDMORE);
			zmq_send(out_socket, &session_ID, sizeof(session_ID), ZMQ_SNDMORE);

			std::map<uint64_t, uint64_t>::iterator sessionIndexPairIT =
					indexBySession.lower_bound(session_ID);
			if (sessionIndexPairIT != indexBySession.end()
					&& !(indexBySession.key_comp()(session_ID,
							sessionIndexPairIT->first))) {
				/*
				 * The session already exists in the map
				 * Generate the actual data to be sent to the client by the proxy
				 */
				std::string response = generateResponse(
						trieByIndex[(*sessionIndexPairIT).second], dataBuffer,
						dataSize);
				zmq_send(out_socket, response.c_str(), response.size(), 0);
			} else {
				/*
				 * The Session does not yet exists in the map
				 * The current message should contain the index
				 */
				if (dataSize != 8) {
					std::cerr
							<< "First message within new Session, which should contain the index, was not 8 byte long!"
							<< dataSize << std::endl;
				} else {
					std::cout << "New Session accessing index "
							<< reinterpret_cast<uint64_t>(dataBuffer)
							<< " connected" << std::endl;
					indexBySession.insert(sessionIndexPairIT,
							std::map<uint64_t, uint64_t>::value_type(session_ID,
									reinterpret_cast<uint64_t>(dataBuffer))); // Use sessionIndexPairIT as a hint to insert
				}
			}

		} else if (strcmp(messageBuffer, "connect") == 0) {
			indexBySession.erase(session_ID);
		} else if (strcmp(messageBuffer, "disconnect") == 0) {
			indexBySession.erase(session_ID);
		}
	}
}
