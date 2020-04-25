#include "http.h"

StringBuffer initStringBuffer() {
	StringBuffer str;
	memset(&str, 0, sizeof(StringBuffer));
	str.buffer = NULL;
	return str;
}

void freeStringBuffer(StringBuffer* str) {
	if (str->buffer != NULL) {
		free(str->buffer);
	}
}

void initHttpModule() {
	curl_global_init(CURL_GLOBAL_ALL);
}

void unloadHttpModule() {
	curl_global_cleanup();
}

size_t curlWriteCallback(void* ptr, size_t size, size_t nmemb, void* stream) {
	StringBuffer* str = (StringBuffer*)stream;
	size_t newLen = str->len + size * nmemb;
	if (str->buffer == NULL) {
		str->buffer = (char*)malloc(newLen + 1);
	} else {
		char* tmp = (char*)realloc(str->buffer, newLen + 1);
		if (tmp != NULL) {
			str->buffer = tmp;
		}
	}
	memcpy(str->buffer + str->len, ptr, size * nmemb);
	str->buffer[newLen] = '\0';
	str->len = newLen;
	return size * nmemb;
}

void curlRequest(char* method, char* host, char* data, char* contentType) {
	CURL* curl;
	CURLcode res;
	StringBuffer body = initStringBuffer();

	curl = curl_easy_init();
	if (!curl) {
		Com_PrintError("^1ERROR failed to init curl\n");
		Scr_AddUndefined();
		return;
	}

	struct curl_slist* headers = NULL;
	if (data != NULL) {
		char* contentTypeHeader = malloc(strlen(contentType) + 14);
		sprintf(contentTypeHeader, "Content-Type: %s", contentType);

		headers = curl_slist_append(headers, contentTypeHeader);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, Scr_GetString(1));
	}

	curl_easy_setopt(curl, CURLOPT_URL, Scr_GetString(0));
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
	res = curl_easy_perform(curl);

	if (res != CURLE_OK) {
		Com_PrintError("^1ERROR sending request: %s\n", curl_easy_strerror(res));
	} else if (body.buffer != NULL) {
		Scr_AddString(body.buffer);
	} else {
		Scr_AddUndefined();
	}

	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);
	freeStringBuffer(&body);
}

// This function fires an HTTP GET request and returns the server's response
void GScr_HttpGet() {
	if (Scr_GetNumParam() != 1) {
		Scr_Error("Usage: httpGet(<url>);\n");
		return;
	}

	curlRequest("GET", Scr_GetString(0), NULL, NULL);
}

// This function fires an HTTP POST request and returns the server's response
void GScr_HttpPost() {
	int params = Scr_GetNumParam();
	if (params < 2 || params > 3) {
		Scr_Error("Usage: httpPost(<url>, <body>, <optional: contentType>);\n");
		return;
	}

	char* contentType = params > 2 ? Scr_GetString(2) : "application/json";
	curlRequest("POST", Scr_GetString(0), Scr_GetString(1), contentType);
}

// This function fires an HTTP PUT request and returns the server's response
void GScr_HttpPut() {
	int params = Scr_GetNumParam();
	if (params < 2 || params > 3) {
		Scr_Error("Usage: httpPut(<url>, <body>, <optional: contentType>);\n");
		return;
	}

	char* contentType = params > 2 ? Scr_GetString(2) : "application/json";
	curlRequest("PUT", Scr_GetString(0), Scr_GetString(1), contentType);
}

// This function fires an HTTP PATCH request and returns the server's response
void GScr_HttpPatch() {
	int params = Scr_GetNumParam();
	if (params < 2 || params > 3) {
		Scr_Error("Usage: httpPatch(<url>, <body>, <optional: contentType>);\n");
		return;
	}

	char* contentType = params > 2 ? Scr_GetString(2) : "application/json";
	curlRequest("PATCH", Scr_GetString(0), Scr_GetString(1), contentType);
}

// This function fires an HTTP DELETE request and returns the server's response
void GScr_HttpDelete() {
	int params = Scr_GetNumParam();
	if (params < 1 || params > 3) {
		Scr_Error("Usage: httpDelete(<url>, <optional: body>, <optional: contentType>);\n");
		return;
	}

	char* body = params > 1 ? Scr_GetString(1) : NULL;
	char* contentType = params > 2 ? Scr_GetString(2) : "application/json";
	curlRequest("DELETE", Scr_GetString(0), body, contentType);
}

// This function fires an HTTP request and returns the server's response
void GScr_HttpRequest() {
	int params = Scr_GetNumParam();
	if (params < 1 || params > 3) {
		Scr_Error("Usage: httpRequest(<method>, <url>, <optional: body>, <optional: contentType>);\n");
		return;
	}

	char* body = params > 1 ? Scr_GetString(2) : NULL;
	char* contentType = params > 2 ? Scr_GetString(3) : "application/json";
	curlRequest(Scr_GetString(0), Scr_GetString(1), body, contentType);
}
