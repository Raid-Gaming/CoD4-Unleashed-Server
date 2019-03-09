#include <string.h>
#include <stdlib.h>

#include "../qcommon_io.h"
#include "../scr_vm.h"

// Argument struct holding data required to send an asynchronous HTTP request
typedef struct {
	// HTTP request info
	char host[64];
	char port[6];
	char path[128];
	char method[16];
	char contentType[128];
	char data[4096];

	// Server response
	char getResponse[1];
	char response[4096];
} AsyncHttpRequestArgs;

enum HttpMethod {
	GET,
	POST,
	PUT,
	PATCH,
	DELETE,
	INVALID
};

void* processHttpRequest(void*);
char* asyncHttpRequest(char* host, int port, char* path, enum HttpMethod method, char* contentType, char* data, int getResponse);

enum HttpMethod strToHttpMethod(char* method);

void GScr_HttpGet();
void GScr_HttpPost();
void GScr_HttpPut();
void GScr_HttpPatch();
void GScr_HttpDelete();
void GScr_HttpRequest();
