#include "connection/Http.hpp"

#include <cstring>
#include <stdexcept>

size_t HTTP::writeDataCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	const size_t recvSize = size * nmemb;
	const auto *dataPtr = static_cast<char *>(contents);
	auto *userMemPtr = static_cast<std::string *>(userp);

	userMemPtr->assign(dataPtr, dataPtr + recvSize);

	return recvSize;
}

HTTP::HTTP(std::string addr, int timeoutInMs) : curl(curl_easy_init()), hostAddr(std::move(addr))
{
	if (curl == nullptr)
	{
		throw std::runtime_error("Can't init curl context");
	}

	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeoutInMs);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, timeoutInMs);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeDataCallback);

	curl_easy_setopt(curl, CURLOPT_SSLENGINE_DEFAULT, 1L);
}

CURLcode HTTP::sendGETRequest(const std::string &index, std::string &receivedData, HttpStatus::Code &statusCode)
{
	// Prepare request specific options
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
	curl_easy_setopt(curl, CURLOPT_URL, (hostAddr + index).c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&receivedData); // Register user-supplied memory

	// Perform request
	long status = static_cast<long>(HttpStatus::Code::xxx_max);
	const CURLcode retval = curl_easy_perform(curl);
	if (retval == CURLE_OK)
	{
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
	}
	statusCode = static_cast<HttpStatus::Code>(status);

	return retval;
}

CURLcode HTTP::sendHEADRequest(const std::string &index, std::string &receivedData, HttpStatus::Code &statusCode)
{
	// Prepare request specific options
	curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
	curl_easy_setopt(curl, CURLOPT_URL, (hostAddr + index).c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&receivedData); // Register user-supplied memory

	// Perform request
	long status = static_cast<long>(HttpStatus::Code::xxx_max);
	const CURLcode retval = curl_easy_perform(curl);
	if (retval == CURLE_OK)
	{
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
	}
	statusCode = static_cast<HttpStatus::Code>(status);

	return retval;
}

CURLcode HTTP::sendPOSTRequest(const std::string &index, const std::string &payload, std::string &receivedData,
							   HttpStatus::Code &statusCode)
{
	// Prepare request specific options
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_URL, (hostAddr + index).c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, payload.size());
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&receivedData);

	// Perform request
	long status = static_cast<long>(HttpStatus::Code::xxx_max);
	const CURLcode retval = curl_easy_perform(curl);
	if (retval == CURLE_OK)
	{
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
	}
	statusCode = static_cast<HttpStatus::Code>(status);

	return retval;
}

CURLcode HTTP::sendPUTRequest(const std::string &index, const std::string &payload, std::string &receivedData,
							  HttpStatus::Code &statusCode)
{
	// Prepare request specific options
	curl_easy_setopt(curl, CURLOPT_URL, (hostAddr + index).c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, payload.size());
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&receivedData);
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

	// Perform request
	long status = static_cast<long>(HttpStatus::Code::xxx_max);
	const CURLcode retval = curl_easy_perform(curl);
	if (retval == CURLE_OK)
	{
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
	}
	statusCode = static_cast<HttpStatus::Code>(status);

	return retval;
}

HTTPStats HTTP::getStats()
{
	HTTPStats stats{};

	curl_off_t value;
	curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD_T, &value);
	stats.uploadBytes = value;
	curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD_T, &value);
	stats.downloadBytes = value;
	curl_easy_getinfo(curl, CURLINFO_HEADER_SIZE, &value);
	stats.headerBytes = value;
	curl_easy_getinfo(curl, CURLINFO_REQUEST_SIZE, &value);
	stats.requestBytes = value;
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
