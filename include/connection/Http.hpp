#pragma once

#include <HttpStatusCodes_C++11.h>
#include <curl/curl.h>

/// HTTP connection timeout in milliseconds
constexpr int HTTP_TIMEOUT_MS = 1000;

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

/**
 * @class HTTP
 * @brief Represents an HTTP client connection
 */
class HTTP {
  private:
	/// CURL handler
	CURL *curl = curl_easy_init();
	/// Full path of server
	std::string hostAddr;

	/**
	 * @brief Sets common fields for HTTP requests
	 * @param[in] fullURL The full URL of the request
	 * @param[out] receivedData The received data from the server
	 * @param[in] method The HTTP method to use
	 */
	void setCommonFields(const std::string &fullURL, std::string &receivedData, CURLoption method);

	/**
	 * @brief Sets common fields for HTTP requests with payload
	 * @param[in] fullURL The full URL of the request
	 * @param[out] receivedData The received data from the server
	 * @param[in] method The HTTP method to use
	 * @param[in] payload The payload to send to the server
	 */
	void setCommonFields(const std::string &fullURL, std::string &receivedData, CURLoption method,
						 const std::string &payload);

	/**
	 * @brief Callback function for writing received data
	 * @param[in] contents The received data
	 * @param[in] size The size of each element
	 * @param[in] nmemb The number of elements
	 * @param[in] userp User pointer
	 * @return The total size of the received data
	 */
	static size_t writeDataCallback(void *contents, size_t size, size_t nmemb, void *userp);

	/**
	 * @brief Performs the request
	 * @param[out] statusCode The HTTP status code (set if CURLE_OK, otherwise unchanged)
	 * @return The status of the operation. CURLE_OK if successful.
	 */
	CURLcode performRequest(HttpStatus::Code &statusCode);

  public:
	/**
	 * @brief Constructs a new HTTP object
	 * @param[in] addr The full path to the server
	 * @param[in] timeoutInMs The connection timeout in milliseconds
	 */
	explicit HTTP(std::string addr, int timeoutInMs = HTTP_TIMEOUT_MS);

	/// @brief Copy constructor
	HTTP(const HTTP & /*unused*/) = delete;

	/// @brief Move constructor
	HTTP(HTTP && /*unused*/) = delete;

	/// @brief Copy assignment operator
	HTTP &operator=(HTTP /*unused*/) = delete;

	/// @brief Move assignment operator
	HTTP &operator=(HTTP && /*unused*/) = delete;

	/**
	 * @brief Sets an option for the HTTP object
	 * @param[in] option The CURL option to set
	 * @param[in] value The corresponding value
	 * @return true if the option was set successfully, false otherwise
	 */
	template <typename T> bool setOption(CURLoption option, T value)
	{
		return curl_easy_setopt(curl, option, value) == CURLE_OK;
	}

	/**
	 * @brief Gets the host address of the object
	 * @return The host address
	 */
	std::string getHostAddress() const { return hostAddr; }

	/**
	 * @brief Sends a GET request
	 * @param[in] index The value to append to the server address
	 * @param[out] receivedData The received reply from the server
	 * @param[out] statusCode The HTTP status code (set if CURLE_OK, otherwise unchanged)
	 * @return The status of the operation. CURLE_OK if successful.
	 */
	CURLcode sendGETRequest(const std::string &index, std::string &receivedData, HttpStatus::Code &statusCode);

	/**
	 * @brief Sends a HEAD request
	 * @param[in] index The value to append to the server address
	 * @param[out] receivedData The received reply from the server
	 * @param[out] statusCode The HTTP status code (set if CURLE_OK, otherwise unchanged)
	 * @return The status of the operation. CURLE_OK if successful.
	 */
	CURLcode sendHEADRequest(const std::string &index, std::string &receivedData, HttpStatus::Code &statusCode);

	/**
	 * @brief Sends a POST request
	 * @param[in] index The value to append to the server address
	 * @param[in] payload The payload to send to the server
	 * @param[out] receivedData The received reply from the server
	 * @param[out] statusCode The HTTP status code (set if CURLE_OK, otherwise unchanged)
	 * @return The status of the operation. CURLE_OK if successful.
	 */
	CURLcode sendPOSTRequest(const std::string &index, const std::string &payload, std::string &receivedData,
							 HttpStatus::Code &statusCode);

	/**
	 * @brief Sends a PUT request
	 * @param[in] index The value to append to the server address
	 * @param[in] payload The payload to send to the server
	 * @param[out] receivedData The received reply from the server
	 * @param[out] statusCode The HTTP status code (set if CURLE_OK, otherwise unchanged)
	 * @return The status of the operation. CURLE_OK if successful.
	 */
	CURLcode sendPUTRequest(const std::string &index, const std::string &payload, std::string &receivedData,
							HttpStatus::Code &statusCode);

	/**
	 * @brief Gets the statistics of the HTTP object
	 * @return The produced statistics
	 */
	HTTPStats getStats();

	/**
	 * @brief Destroys the HTTP object
	 */
	~HTTP();
};
