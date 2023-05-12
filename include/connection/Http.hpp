#pragma once

#include <HttpStatusCodes_C++11.h>
#include <curl/curl.h>

/**
 * @brief Stats produced by HTTP
 */
struct HTTPStats {
	/// Total uploaded bytes
	size_t uploadBytes;
	/// Total downloaded bytes
	size_t downloadBytes;
	/// Total header size in bytes
	size_t headerBytes;
	/// Total request size in bytes
	size_t requestBytes;
	/// Upload bandwidth in Bps
	long uploadSpeed;
	/// Download bandwidth in Bps
	long downloadSpeed;
	/// Total processing time for connection in microseconds
	long connectionTime;
	/// Total processing time for name lookup in microseconds
	long nameLookupTime;
	/// Total processing time for pre-transfer period in microseconds
	long preTransferTime;
	/// Total processing time for redirections in microseconds
	long redirectTime;
	/// Total processing time for starting transfer in microseconds
	long startTransferTime;
	/// Total processing time in microseconds
	long totalTime;
};

class HTTP {
  private:
	/// CURL handler
	CURL *curl;
	/// Full path of server
	std::string hostAddr;

	void setCommonFields(const std::string &fullURL, std::string &receivedData, CURLoption method);
	void setCommonFields(const std::string &fullURL, std::string &receivedData, CURLoption method,
						 const std::string &payload);
	CURLcode performRequest(HttpStatus::Code &statusCode);

	static size_t writeDataCallback(void *contents, size_t size, size_t nmemb, void *userp);

  public:
	/**
	 * @brief Constructs a new HTTP class
	 * @param[in] addr Full path to server
	 * @param[in] timeoutInMs Connection timeout in milliseconds
	 */
	explicit HTTP(std::string addr, int timeoutInMs = 1000);

	/// @brief Copy constructor
	HTTP(const HTTP & /*unused*/) = delete;

	/// @brief Move constructor
	HTTP(HTTP && /*unused*/) = delete;

	/// @brief Copy assignment operator
	HTTP &operator=(HTTP /*unused*/) = delete;

	/// @brief Move assignment operator
	HTTP &operator=(HTTP && /*unused*/) = delete;

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
	 * @brief Get the host address of the object
	 * @return std::string Host address
	 */
	std::string getHostAddress() { return hostAddr; }

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
	 * @param[in] index Value to append to server address
	 * @param[out] receivedData Received reply from server
	 * @param[out] statusCode HTTP status code. Set if CURLE_OK, otherwise unchanged
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
	 * @brief Get the statistics of class
	 * @return HTTPStats Produced statistics
	 */
	HTTPStats getStats();

	/**
	 * @brief Destroys the HTTP object
	 */
	~HTTP();
};
