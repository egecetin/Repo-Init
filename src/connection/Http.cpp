#include "connection/Http.hpp"

#include <cstring>
#include <stdexcept>

void HTTP::setCommonFields(const std::string &fullURL, std::string &receivedData, CURLoption method)
{
	curl_easy_setopt(curl, CURLOPT_URL, fullURL.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, static_cast<void *>(&receivedData)); // Register user-supplied memory
	curl_easy_setopt(curl, method, 1L);
}

void HTTP::setCommonFields(const std::string &fullURL, std::string &receivedData, CURLoption method,
						   const std::string &payload)
{
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, payload.size());
	curl_easy_setopt(curl, CURLOPT_URL, fullURL.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, static_cast<void *>(&receivedData)); // Register user-supplied memory
	curl_easy_setopt(curl, method, 1L);
}

CURLcode HTTP::performRequest(HttpStatus::Code &statusCode)
{
	// Perform request
	auto status = static_cast<long>(HttpStatus::Code::xxx_max);
	const CURLcode retval = curl_easy_perform(curl);
	if (retval == CURLE_OK)
	{
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
	}
	statusCode = static_cast<HttpStatus::Code>(status);

	return retval;
}

size_t HTTP::writeDataCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	const size_t recvSize = size * nmemb;
	const auto *dataPtr = static_cast<char *>(contents);
	auto *userMemPtr = static_cast<std::string *>(userp);

	userMemPtr->assign(dataPtr, dataPtr + recvSize);

	return recvSize;
}

HTTP::HTTP(std::string addr, int timeoutInMs) : hostAddr(std::move(addr))
{
	if (curl == nullptr)
	{
		throw std::invalid_argument("Can't init curl context");
	}

	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeoutInMs);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, timeoutInMs);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeDataCallback);

	curl_easy_setopt(curl, CURLOPT_SSLENGINE_DEFAULT, 1L);
	curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2); // At least TLSv1.2
}

CURLcode HTTP::sendGETRequest(const std::string &index, std::string &receivedData, HttpStatus::Code &statusCode)
{
	// Prepare request specific options
	setCommonFields(hostAddr + index, receivedData, CURLOPT_HTTPGET);
	return performRequest(statusCode);
}

CURLcode HTTP::sendHEADRequest(const std::string &index, std::string &receivedData, HttpStatus::Code &statusCode)
{
	// Prepare request specific options
	setCommonFields(hostAddr + index, receivedData, CURLOPT_NOBODY);
	return performRequest(statusCode);
}

CURLcode HTTP::sendPOSTRequest(const std::string &index, const std::string &payload, std::string &receivedData,
							   HttpStatus::Code &statusCode)
{
	// Prepare request specific options
	setCommonFields(hostAddr + index, receivedData, CURLOPT_POST, payload);
	return performRequest(statusCode);
}

CURLcode HTTP::sendPUTRequest(const std::string &index, const std::string &payload, std::string &receivedData,
							  HttpStatus::Code &statusCode)
{
	// Prepare request specific options
	setCommonFields(hostAddr + index, receivedData, CURLOPT_UPLOAD, payload);
	return performRequest(statusCode);
}

HTTPStats HTTP::getStats()
{
	HTTPStats stats{};

	curl_off_t value = 0;
	curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD_T, &value);
	stats.uploadBytes = static_cast<size_t>(value);
	curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD_T, &value);
	stats.downloadBytes = static_cast<size_t>(value);
	curl_easy_getinfo(curl, CURLINFO_HEADER_SIZE, &value);
	stats.headerBytes = static_cast<size_t>(value);
	curl_easy_getinfo(curl, CURLINFO_REQUEST_SIZE, &value);
	stats.requestBytes = static_cast<size_t>(value);
	curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD_T, &value);
	stats.uploadSpeed = value;
	curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD_T, &value);
	stats.downloadSpeed = value;
	curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME_T, &value);
	stats.connectionTime = value;
	curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME_T, &value);
	stats.nameLookupTime = value;
	curl_easy_getinfo(curl, CURLINFO_PRETRANSFER_TIME_T, &value);
	stats.preTransferTime = value;
	curl_easy_getinfo(curl, CURLINFO_REDIRECT_TIME_T, &value);
	stats.redirectTime = value;
	curl_easy_getinfo(curl, CURLINFO_STARTTRANSFER_TIME_T, &value);
	stats.startTransferTime = value;
	curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &value);
	stats.totalTime = value;

	return stats;
}

HTTP::~HTTP() { curl_easy_cleanup(curl); }
