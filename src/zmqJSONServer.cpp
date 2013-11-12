//
//  Hello World server in C
//  Binds REP socket to tcp://*:5555
//  Expects "Hello" from client, replies with "World"
//

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <zmq.h>
//
//static int receiveString(void *socket, const unsigned short length,
//		char* buffer) {
//	int size = zmq_recv(socket, buffer, length, 0);
//	if (size == -1)
//		return size;
//	if (size > length)
//		size = length;
//
//	buffer[size] = '\0';
//	return size;
//}
//static void generateResponse(char* request, char* response) {
//	strcpy(response,
//			"{\"suggestionList\": [{\"key\": 1, \"suggestion\": \"Anna\"}, {\"key\": 2, \"suggestion\": \"Berta\"}, {\"key\": 3, \"suggestion\": \"Carolin\"}, {\"key\": 4, \"suggestion\": \"Dorothee\"}, {\"key\": \"\", \"suggestion\": \"df\"}]}");
//}
//
//static int startServer(void) {
//	/*
//	 * Connect to pull and push socket of sockjsproxy
//	 */
//	void *context = zmq_ctx_new();
//	void *in_socket = zmq_socket(context, ZMQ_PULL);
//	zmq_connect(in_socket, "tcp://localhost:9241");
//	void *out_socket = zmq_socket(context, ZMQ_PUSH);
//	zmq_connect(out_socket, "tcp://localhost:9242");
//
//	char messageBuffer[13];
//	char dataBuffer[1500];
//	while (1) {
//		int messageSize = receiveString(in_socket, sizeof(messageBuffer),
//				&messageBuffer[0]);
//		uint64_t session_ID;
//		zmq_recv(in_socket, &session_ID, sizeof(session_ID), 0);
//
//		receiveString(in_socket, sizeof(dataBuffer), dataBuffer);
//
//		if (strcmp(messageBuffer, "message\0") == 0) {
//			printf("message: %s\n", messageBuffer);
//			printf("data: %s\n", dataBuffer);
//			zmq_send(out_socket, messageBuffer, messageSize, ZMQ_SNDMORE);
//			zmq_send(out_socket, &session_ID, sizeof(session_ID), ZMQ_SNDMORE);
//			char response[1500];
//			generateResponse(dataBuffer, response);
//			zmq_send(out_socket, response, strlen(response), 0);
//		} else if (strcmp(messageBuffer, "connect\0") == 0) {
//			printf("New client: %ld\n", session_ID);
//		} else if (strcmp(messageBuffer, "disconnect\0") == 0) {
//			printf("Client disconnected: %ld\n", session_ID);
//		}
//	}
//	return 0;
//}
