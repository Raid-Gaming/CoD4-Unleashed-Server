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

void curlRequest(char* method, char* host, char* data, unsigned int startOfContentArgs) {
	CURL* curl;
	CURLcode res;
	StringBuffer body = initStringBuffer();

	curl = curl_easy_init();
	if (!curl) {
		Com_PrintError("^1ERROR failed to init curl\n");
		Scr_AddUndefined();
		return;
	}

	int params = Scr_GetNumParam();

	curl_easy_setopt(curl, CURLOPT_URL, Scr_GetString(startOfContentArgs));
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);

	if (data != NULL) {
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, Scr_GetString(startOfContentArgs + 1));
	} else {
		startOfContentArgs -= 1;
	}

	struct curl_slist* headers = NULL;
	unsigned int startOfHeaders = startOfContentArgs + 2;
	if (params > startOfHeaders) {
		int i;
		for (i = startOfHeaders; i < params; ++i) {
			if (Scr_GetType(i) == 2) {
				headers = curl_slist_append(headers, Scr_GetString(i));
			}
		}
	} else {
		headers = curl_slist_append(headers, "Content-Type: application/json");
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

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
	if (Scr_GetNumParam() < 1) {
		Scr_Error("Usage: httpGet(<url>, <optional: headers>...);\n");
		return;
	}

	curlRequest("GET", Scr_GetString(0), NULL, 0);
}

// This function fires an HTTP POST request and returns the server's response
void GScr_HttpPost() {
	int params = Scr_GetNumParam();
	if (params < 2) {
		Scr_Error("Usage: httpPost(<url>, <body>, <optional: headers>...);\n");
		return;
	}

	curlRequest("POST", Scr_GetString(0), Scr_GetString(1), 0);
}

// This function fires an HTTP PUT request and returns the server's response
void GScr_HttpPut() {
	int params = Scr_GetNumParam();
	if (params < 2) {
		Scr_Error("Usage: httpPut(<url>, <body>, <optional: headers>...);\n");
		return;
	}

	curlRequest("PUT", Scr_GetString(0), Scr_GetString(1), 0);
}

// This function fires an HTTP PATCH request and returns the server's response
void GScr_HttpPatch() {
	int params = Scr_GetNumParam();
	if (params < 2) {
		Scr_Error("Usage: httpPatch(<url>, <body>, <optional: headers>...);\n");
		return;
	}

	curlRequest("PATCH", Scr_GetString(0), Scr_GetString(1), 0);
}

// This function fires an HTTP DELETE request and returns the server's response
void GScr_HttpDelete() {
	int params = Scr_GetNumParam();
	if (params < 1) {
		Scr_Error("Usage: httpDelete(<url>, <optional: body>, <optional: headers>...);\n");
		return;
	}

	char* body = params > 1 && Scr_GetType(1) == 2 ? Scr_GetString(1) : NULL;
	curlRequest("DELETE", Scr_GetString(0), body, 0);
}

// This function fires an HTTP request and returns the server's response
void GScr_HttpRequest() {
	int params = Scr_GetNumParam();
	if (params < 2) {
		Scr_Error("Usage: httpRequest(<method>, <url>, <optional: body>, <optional: headers>...);\n");
		return;
	}

	char* body = params > 2 && Scr_GetType(2) == 2 ? Scr_GetString(2) : NULL;
	curlRequest(Scr_GetString(0), Scr_GetString(1), body, 1);
}
