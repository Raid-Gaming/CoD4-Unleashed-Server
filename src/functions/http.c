#include "http.h"

#if defined(_WIN32) || defined(_MSC_VER)
// TODO: Implement win32 version using WinSock
#else

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

#endif

#if defined(_WIN32) || defined(_MSC_VER)

// No win32 implementation yet
char* asyncHttpRequest(char* host, int port, char* path, enum HttpMethod method, char* contentType, char* data, int getResponse) {
	return "";
}

#else

void* processHttpRequest(void* args) {
	AsyncHttpRequestArgs* argStruct = args;

	struct hostent* server;
	struct sockaddr_in servAddr;
	int sockfd, bytes, sent, total;
	char* msg, message[5130], response[4096];

	// HTTP header
	sprintf(message, "%s %s HTTP/1.1\r\n", argStruct->method, argStruct->path);
	sprintf(message + strlen(message), "Host: %s:%s\r\n", argStruct->host, argStruct->port);

	// GET cannot have a body, only append data if method is not GET
	if (strcmp(argStruct->method, "GET") != 0 && strlen(argStruct->data) > 0) {
		sprintf(message + strlen(message), "Content-Type: %s\r\n", argStruct->contentType);
		sprintf(message + strlen(message), "Content-Length: %d\r\n", strlen(argStruct->data));
		strcat(message, "\r\n");
		strcat(message, argStruct->data);
	} else {
		strcat(message, "\r\n");
	}

	// Create the socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		Com_PrintError("^1EROR opening socket");
		close(sockfd);
		pthread_exit(NULL);
	}

	// Lookup the ip address
	server = gethostbyname(argStruct->host);
	if (server == NULL) {
		Com_PrintError("^1ERROR, no such host");
		close(sockfd);
		pthread_exit(NULL);
	}

	// Fill in the structure
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(atoi(argStruct->port));
	memcpy(&servAddr.sin_addr.s_addr, server->h_addr, server->h_length);

	// Connect to socket
	if (connect(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
		Com_PrintError("^1ERROR connecting to socket");
		close(sockfd);
		pthread_exit(NULL);
	}

	msg = message;

	// Send the request
	total = strlen(msg);
	sent = 0;
	do {
		bytes = write(sockfd, msg + sent, total - sent);
		if (bytes < 0) {
			Com_PrintError("^1ERROR writing message to socket");
			close(sockfd);
			pthread_exit(NULL);
		}

		if (bytes == 0) {
			break;
		}

		sent += bytes;
	} while (sent < total);

	if (*argStruct->getResponse == '1') {
		// Receive the response
		memset(response, 0, sizeof(response));
		total = sizeof(response) - 1;

		bytes = read(sockfd, response, total);
		if (bytes < 0) {
			Com_PrintError("^1ERROR reading response from socket");
			close(sockfd);
			pthread_exit(NULL);
		}

		if (bytes == total) {
			Com_PrintError("^1ERROR storing complete response from socket");
			close(sockfd);
			pthread_exit(NULL);
		}

		// Strip the header from the response
		char* const endof = strstr(response, "\r\n\r\n");
		*endof = '\0';

		strcpy(argStruct->response, endof + 4);
	}

	close(sockfd);
	pthread_exit(NULL);
}

char* asyncHttpRequest(char* host, int port, char* path, enum HttpMethod method, char* contentType, char* data, int getResponse) {
	pthread_t thread;
	char* response;

	// Allocate needed memory to hold the data
	AsyncHttpRequestArgs *args = malloc(sizeof(AsyncHttpRequestArgs));

	// Fill out the structure
	strcpy(args->host, host);
	sprintf(args->port, "%d", port);
	strcpy(args->path, path);
	strcpy(args->contentType, contentType);
	if (method != GET) {
		strcpy(args->data, data);
	}
	sprintf(args->getResponse, "%d", getResponse);
	
	switch (method) {
	case GET:
		strcpy(args->method, "GET");
		break;

	case POST:
		strcpy(args->method, "POST");
		break;

	case PUT:
		strcpy(args->method, "PUT");
		break;

	case PATCH:
		strcpy(args->method, "PATCH");
		break;

	case DELETE:
		strcpy(args->method, "DELETE");
		break;
	
	default:
		free(args);
		Scr_Error("Invalid HTTP 1.1 method, use GET/POST/PUT/PATCH/DELETE\n");
		return "";
	}
	
	// Create a thread handling the HTTP request
	pthread_create(&thread, NULL, processHttpRequest, args);

	// Wait for the thread to end
	pthread_join(thread, NULL);

	// Copy response from struct into response variable
	if (getResponse == 1) {
		response = args->response;
	} else {
		response = "";
	}

	// Free allocated memory after it's no longer needed
	free(args);

	// Return server response
	return response;
}

#endif

enum HttpMethod strToHttpMethod(char* method) {
	if (strcmp(method, "GET") == 0) {
		return GET;
	} else if (strcmp(method, "POST") == 0) {
		return POST;
	} else if(strcmp(method, "PUT") == 0) {
		return PUT;
	} else if (strcmp(method, "PATCH") == 0) {
		return PATCH;
	} else if(strcmp(method, "DELETE") == 0) {
		return DELETE;
	} else {
		return INVALID;
	}
}

/*
This function fires an HTTP GET request and returns the server's response
You can optionally set the receive argument to false to not receive any responses
*/
void GScr_HttpGet() {
	int params = Scr_GetNumParam();

	if (params < 3 || params > 4) {
		Scr_Error("Usage: httpGet(<host>, <port>, <path>, <optional: receive>)\n");
		return;
	}

	char* host = Scr_GetString(0);
	int port = Scr_GetInt(1);
	char* path = Scr_GetString(2);

	Scr_AddString(asyncHttpRequest(
		host,
		port,
		path,
		GET,
		"application/json",
		"",
		params == 4 ? Scr_GetInt(3) : 1
	));
}

/*
This function fires an HTTP POST request and returns the server's response
You can optionally set the receive argument to false to not receive any responses
*/
void GScr_HttpPost() {
	int params = Scr_GetNumParam();

	if (params < 4 || params > 6) {
		Scr_Error("Usage: httpPost(<host>, <port>, <path>, <data>, <optional: contentType>, <optional: receive>)\n");
		return;
	}

	char* host = Scr_GetString(0);
	int port = Scr_GetInt(1);
	char* path = Scr_GetString(2);
	char* data = Scr_GetString(3);

	Scr_AddString(asyncHttpRequest(
		host,
		port,
		path,
		POST,
		params > 4 ? Scr_GetString(4) : "application/json",
		data,
		params == 6 ? Scr_GetInt(5) : 1
	));
}

/*
This function fires an HTTP PUT request and returns the server's response
You can optionally set the receive argument to false to not receive any responses
*/
void GScr_HttpPut() {
	int params = Scr_GetNumParam();

	if (params < 4 || params > 6) {
		Scr_Error("Usage: httpPut(<host>, <port>, <path>, <data>, <optional: contentType>, <optional: receive>)\n");
		return;
	}

	char* host = Scr_GetString(0);
	int port = Scr_GetInt(1);
	char* path = Scr_GetString(2);
	char* data = Scr_GetString(3);

	Scr_AddString(asyncHttpRequest(
		host,
		port,
		path,
		PUT,
		params > 4 ? Scr_GetString(4) : "application/json",
		data,
		params == 6 ? Scr_GetInt(5) : 1
	));
}

/*
This function fires an HTTP PATCH request and returns the server's response
You can optionally set the receive argument to false to not receive any responses
*/
void GScr_HttpPatch() {
	int params = Scr_GetNumParam();

	if (params < 4 || params > 6) {
		Scr_Error("Usage: httpPatch(<host>, <port>, <path>, <data>, <optional: contentType>, <optional: receive>)\n");
		return;
	}

	char* host = Scr_GetString(0);
	int port = Scr_GetInt(1);
	char* path = Scr_GetString(2);
	char* data = Scr_GetString(3);

	Scr_AddString(asyncHttpRequest(
		host,
		port,
		path,
		PATCH,
		params > 4 ? Scr_GetString(4) : "application/json",
		data,
		params == 6 ? Scr_GetInt(5) : 1
	));
}

/*
This function fires an HTTP DELETE request and returns the server's response
You can optionally set the receive argument to false to not receive any responses
*/
void GScr_HttpDelete() {
	int params = Scr_GetNumParam();

	if (params < 3 || params > 6) {
		Scr_Error("Usage: httpDelete(<host>, <port>, <path>, <optional: data>, <optional: contentType>, <optional: receive>)\n");
		return;
	}

	char* host = Scr_GetString(0);
	int port = Scr_GetInt(1);
	char* path = Scr_GetString(2);
	char* data = params > 3 ? Scr_GetString(3) : "";

	Scr_AddString(asyncHttpRequest(
		host,
		port,
		path,
		DELETE,
		params > 4 ? Scr_GetString(4) : "application/json",
		data,
		params == 6 ? Scr_GetInt(5) : 1
	));
}

/*
This function fires an HTTP request and returns the server's response
You can optionally set the receive argument to false to not receive any responses
*/
void GScr_HttpRequest() {
	int params = Scr_GetNumParam();

	if (params < 4 || params > 7) {
		Scr_Error("Usage: httpRequest(<method>, <host>, <port>, <path>, <optional: data>, <optional: contentType>, <optional: receive>)\n");
		return;
	}

	char* host = Scr_GetString(1);
	int port = Scr_GetInt(2);
	char* path = Scr_GetString(3);
	char* data = params > 4 ? Scr_GetString(4) : "";

	Scr_AddString(asyncHttpRequest(
		host,
		port,
		path,
		strToHttpMethod(Scr_GetString(0)),
		params > 5 ? Scr_GetString(5) : "application/json",
		data,
		params == 7 ? Scr_GetInt(6) : 1
	));
}
