#include "connection/Http.hpp"

#include <stdexcept>

#include <spdlog/spdlog.h>

struct MemoryStruct_t
{
	char *memory;
	size_t size;
};
typedef struct MemoryStruct_t MemoryStruct;

size_t HTTP::writeDataCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	MemoryStruct *mem = (MemoryStruct *)userp;

	char *ptr = (char *)realloc(mem->memory, mem->size + realsize + 1);
	if (!ptr)
		throw std::runtime_error("Out of memory (realloc returned NULL)");

	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

HTTP::HTTP(const std::string &addr, int timeoutInMs)
{
	hostAddr = addr;
	curl = curl_easy_init();
	if (!curl)
		throw std::runtime_error("Can't init curl context");

	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeoutInMs);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, timeoutInMs);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeDataCallback);

	curl_easy_setopt(curl, CURLOPT_SSLENGINE_DEFAULT, 1L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
}

CURLcode HTTP::sendPOSTRequest(const std::string &index, const std::string &payload, std::string &receivedData,
							   long &statusCode)
{
	// Prepare memory
	MemoryStruct chunk;
	chunk.size = 0;
	chunk.memory = (char *)malloc(1);
	if (!(chunk.memory))
		throw std::runtime_error("Out of memory (malloc returned NULL)");

	// Prepare request specific options
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_URL, (hostAddr + index).c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, payload.size());
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

	// Perform request
	CURLcode retval = curl_easy_perform(curl);
	if (retval == CURLE_OK)
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
	receivedData = std::string(chunk.memory, chunk.size);

	// Cleanup
	free(chunk.memory);

	return retval;
}

CURLcode HTTP::sendGETRequest(const std::string &index, std::string &receivedData, long &statusCode)
{
	// Prepare memory
	MemoryStruct chunk;
	chunk.size = 0;
	chunk.memory = (char *)malloc(1);
	if (!(chunk.memory))
		throw std::runtime_error("Out of memory (malloc returned NULL)");

	// Prepare request specific options
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
	curl_easy_setopt(curl, CURLOPT_URL, (hostAddr + index).c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk); // Register user-supplied memory

	// Perform request
	CURLcode retval = curl_easy_perform(curl);
	if (retval == CURLE_OK)
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
	receivedData = std::string(chunk.memory, chunk.size);

	// Cleanup
	free(chunk.memory);

	return retval;
}

HTTP::~HTTP() { curl_easy_cleanup(curl); }
