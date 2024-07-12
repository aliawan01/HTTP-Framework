#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#pragma comment(lib, "Ws2_32.lib")

enum RequestMethodType {
	GET,
	PUT,
	POST,
	DELETE_REQUEST, // Some header file made a macro defining DELETE as ).
	UKNOWN
};

enum RequestMethodType parseRequest(char* requestBuffer) {
	char requestMethodBuffer[20] = {0};

	for (int index = 0; requestBuffer[index] != ' '; index++) {
		requestMethodBuffer[index] = requestBuffer[index];
	}

	if (!strcmp(requestMethodBuffer, "GET")) {
		return GET;
	} 
	else {
		return UKNOWN;
	}
}

char* getRequestedRoute(char* requestBuffer) {
	char* requestMethodBuffer = malloc(500);
	memset(requestMethodBuffer, 0, 500);

	int index = 0;
	for (;requestBuffer[index] != ' '; index++) {}
	index++;


	for (int secondIndex = 0;requestBuffer[index] != ' '; index++, secondIndex++) {
		requestMethodBuffer[secondIndex] = requestBuffer[index];
	}

	return requestMethodBuffer;
}

int getFileSize(char* fileName) {
	FILE* file = fopen(fileName, "rb");
	if (file == NULL) {
		printf("getFileSize(): Error couldn't open the file %s\n", fileName);
		return -1;
	}

	fseek(file, 0, SEEK_END);
	int fileSize = ftell(file);
	fclose(file);
	return fileSize;
}

char* getFileContents(char* fileName) {
	FILE* file = fopen(fileName, "rb");
	if (file == NULL) {
		printf("getFileContents(): Error couldn't open the file %s\n", fileName);
		// TODO(ali): Fix this.
		return "didn't work";
	}

	int fileSize = getFileSize(fileName);


	char* fileContents = malloc(fileSize+1);
	memset(fileContents, 0, fileSize+1);

	fread(fileContents, sizeof(char), fileSize, file);
	fclose(file);
	return fileContents;
}

int main() {
	const char* serverIPAddress = "127.0.0.1";
	const char* serverPort = "8000";

	WSADATA* wsaData;
	int initResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (initResult != 0) {
		printf("[SERVER] WSAStartup failed: %d\n", initResult);
		return 1;
	}

	struct addrinfo* result = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	initResult = getaddrinfo(serverIPAddress, serverPort, &hints, &result);
	if (initResult != 0) {
		printf("[SERVER] getaddrinfo failed: %d\n", initResult);
		WSACleanup();
		return 1;
	}

	SOCKET listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listenSocket == INVALID_SOCKET) {
		printf("[SERVER] Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	initResult = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (initResult == SOCKET_ERROR) {
		printf("[SERVER] bind() failed with the error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	if (listen(listenSocket, 5) == SOCKET_ERROR) {
		printf("[SERVER] listen() failed with error: %ld\n", WSAGetLastError());
	}
	else {
		printf("[SERVER] is listening on IP Address: %s, and Port Number: %s\n", serverIPAddress, serverPort);
	}

	while (true) {
		SOCKET clientSocket = accept(listenSocket, NULL, NULL);
		if (clientSocket == INVALID_SOCKET) {
			printf("[SERVER] accept() failed: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}

		char recevingBuffer[2056];
		int recevingBufferLen = 2056;
		int initSendResult = 1;

		initResult = recv(clientSocket, recevingBuffer, recevingBufferLen, 0);
		if (initResult > 0) {
			printf("[SERVER] Bytes received: %d\n", initResult);
			printf("[SERVER] Data Received: %s\n", recevingBuffer);

			enum RequestMethodType requestType = parseRequest(recevingBuffer);
			switch (requestType) {
				case GET:
					printf("Parse URI: GET\n");
					break;
				case PUT:
					printf("Parse URI: PUT\n");
					break;
				case POST:
					printf("Parse URI: POST\n");
					break;
				case DELETE_REQUEST:
					printf("Parse URI: DELETE\n");
					break;
				default:
					printf("Parse URI: Couldn't figure it out.\n");
			}
			char* requestedRoute = getRequestedRoute(recevingBuffer);
			printf("Requested Route: %s\n", requestedRoute);

			char* htmlCode;
			unsigned char* httpResponseHeader;
			bool showingImage = false;
			int htmlCodeLength;

			if (!strcmp(requestedRoute, "/favicon.ico")) {
				printf("----------------------\n");
				printf("Asked for a favicon!!!\n");
				printf("----------------------\n");

				htmlCode = getFileContents("static/favicon.ico");
				htmlCodeLength = getFileSize("static/favicon.ico");
				httpResponseHeader = "HTTP/1.1 200 OK\r\nContent-Type: image/x-icon\r\n\r\n";
				showingImage = true;
			}
			else if (!strcmp(requestedRoute, "/ginger.jpg")) {
				printf("----------------------\n");
				printf("Asked for ginger.jpg!!!!!\n");
				printf("----------------------\n");

				htmlCode = getFileContents("static/ginger.jpg");
				htmlCodeLength = getFileSize("static/ginger.jpg");
				httpResponseHeader = "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\n\r\n";
				showingImage = true;
			}
			else {
				htmlCode = getFileContents("static/firstPage.html");
				htmlCodeLength = strlen(htmlCode);
				httpResponseHeader = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
			}

			printf("Html Code: %s\n", htmlCode);

			int httpResponseHeaderLength = (int)strlen(httpResponseHeader);

			send(clientSocket, httpResponseHeader, httpResponseHeaderLength, 0);
			send(clientSocket, htmlCode, htmlCodeLength, 0);

			printf("[SERVER] Bytes sent: %d\n", initSendResult);
			printf("[SERVER] Data sent (HEADER): %s\n", httpResponseHeader);
			printf("[SERVER] Data sent (DATA): %s\n", htmlCode);

			free(htmlCode);
			free(requestedRoute);
		}
		else if (initResult == 0) {
			printf("[SERVER] Connection gracefully closing...\n");
		}
		else {
			printf("[SERVER] recv failed: %d\n", WSAGetLastError());
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}

		closesocket(clientSocket);
	}

	/*
	initResult = shutdown(clientSocket, SD_SEND);
	if (initResult == SOCKET_ERROR) {
		printf("[SERVER] shutdown() failed: %d\n", WSAGetLastError());
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}

	closesocket(clientSocket);
	WSACleanup();
	*/

	return 0;
}
