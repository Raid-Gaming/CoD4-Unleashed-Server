#include <stdlib.h>
#include <curl/curl.h>

#include "../qcommon_io.h"
#include "../scr_vm.h"

typedef struct {
    char* buffer;
    size_t len;
} StringBuffer;

StringBuffer initStringBuffer();
void freeStringBuffer(StringBuffer*);

void initHttpModule();
void unloadHttpModule();

size_t curlWriteCallback(void*, size_t, size_t, void*);

void curlRequest(char*, char*, char*, unsigned int);

void GScr_HttpGet();
void GScr_HttpPost();
void GScr_HttpPut();
void GScr_HttpPatch();
void GScr_HttpDelete();
void GScr_HttpRequest();
