#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "curl/curl.h"
#include "mycurl.h"
#include "log.h"
typedef struct MemoryStruct {
  size_t size;
  char *memory;
} MemoryStruct;
#define CURL_TAG "Curl: "
size_t writeMemoryCallback(void *contents, size_t size, size_t nmemb,
        MemoryStruct *userp)
{
  size_t realsize = size * nmemb;
  MemoryStruct *mem = (MemoryStruct *)userp;
  mem->memory = (char *)malloc(realsize);
  memcpy(mem->memory, contents, realsize);
  mem->size = realsize;
  //printfLog(CURL_TAG"writeMemoryCallback:%s", (char *)contents);
  return realsize;
}
int send_http_post(char * url, char * param, void * callBack,
		void * callBackHandle)
{
	int ret = -1;
	static CURL *curl_handle = NULL;
	CURLcode res;
	//config.trace_ascii = 1;
	//printfLog(CURL_TAG"send_http_post ==>\n");
	//printfLog(CURL_TAG"send_http_post url:%s\n", url);

	if (curl_handle == NULL) {
		curl_handle = curl_easy_init();
		printfLog(CURL_TAG"send_http_post first in %p\n", curl_handle);
	}

	curl_easy_setopt(curl_handle, CURLOPT_URL, 			url);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION,callBack);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, 	callBackHandle);
	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 	1);
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 		5);
	curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 		0L);
	curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 	1L);
	curl_easy_setopt(curl_handle, CURLOPT_PROTOCOLS, 	CURLPROTO_HTTP);
	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, 	param);
	curl_easy_setopt(curl_handle, CURLOPT_POST, 		1);

	res = curl_easy_perform(curl_handle);

	if(res != CURLE_OK) {
		printfLog(CURL_TAG"send_http_post: failed: %s\n", curl_easy_strerror(res));
	} else {
		ret = 0;
	}

	//curl_easy_cleanup(curl_handle);

	//printfLog(CURL_TAG"send_http_post <==\n");
	return ret;
}
char* http_post(char* url, char* request, int timeout)
{

    MemoryStruct mem = {0, NULL};
	printfLog(CURL_TAG"argv[1] %s\n", url);
	printfLog(CURL_TAG"argv[2] %s\n", request);
	int ret = send_http_post(url, request,
			(void *)writeMemoryCallback, (void *)&mem);
	if(0 != ret) {
		printfLog(CURL_TAG"send_status_req send_http_get error\n");
	}
	else
	{
		printfLog(CURL_TAG"server response: %s\n", mem.memory);
	}
	return mem.memory;
}

