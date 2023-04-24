#pragma once

#include <string>

#include <HttpStatusCodes_C++11.h>
#include <curl/curl.h>

class HTTP
{
  private:
	// CURL handler
	CURL *curl;
	// Full path of server
	std::string hostAddr;

	static size_t writeDataCallback(void *contents, size_t size, size_t nmemb, void *userp);

  public:
	/**
	 * @brief Constructs a new HTTP class
	 * @param[in] addr Full path to server
	 * @param[in] timeoutInMs Connection timeout in milliseconds
	 */
	explicit HTTP(const std::string &addr, int timeoutInMs = 1000);

	/**
	 * @brief Set the option of HTTP class
	 * @param[in] option CURL option
	 * @param[in] value Corresponding value
	 * @return true If successful
	 * @return false Otherwise
	 */
	template <typename T> bool setOption(CURLoption option, T value)
	{
		return curl_easy_setopt(curl, option, value) == CURLE_OK;
	}

	/**
	 * @brief Sends a GET request
	 * @param[in] index Value to append to server address
	 * @param[in] receivedData Received reply from server
	 * @param[out] statusCode HTTP status code, Set if CURLE_OK, otherwise unchanged
	 * @return CURLcode Status of operation. CURLE_OK if successful.
	 */
	CURLcode sendGETRequest(const std::string &index, std::string &receivedData, HttpStatus::Code &statusCode);

	/**
	 * @brief Sends a HEAD request
	 * @param index Value to append to server address
	 * @param receivedData Received reply from server
	 * @param statusCode HTTP status code. Set if CURLE_OK, otherwise unchanged
	 * @return CURLcode Status of operation. CURLE_OK if successful.
	 */
	CURLcode sendHEADRequest(const std::string &index, std::string &receivedData, HttpStatus::Code &statusCode);

	/**
	 * @brief Sends a POST request
	 * @param[in] index Value to append to server address
	 * @param[in] payload Payload to send to server
	 * @param[out] receivedData Received reply from server
	 * @param[out] statusCode HTTP status code. Set if CURLE_OK, otherwise unchanged
	 * @return CURLcode Status of operation. CURLE_OK if successful.
	 */
	CURLcode sendPOSTRequest(const std::string &index, const std::string &payload, std::string &receivedData,
							 HttpStatus::Code &statusCode);

	/**
	 * @brief Sends a PUT request
	 * @param[in] index Value to append to server address
	 * @param[in] payload Payload to send to server
	 * @param[out] receivedData Received reply from server
	 * @param[out] statusCode HTTP status code. Set if CURLE_OK, otherwise unchanged
	 * @return CURLcode Status of operation. CURLE_OK if successful.
	 */
	CURLcode sendPUTRequest(const std::string &index, const std::string &payload, std::string &receivedData,
							HttpStatus::Code &statusCode);

	/**
	 * @brief Destroys the HTTP object
	 */
	~HTTP();
};
