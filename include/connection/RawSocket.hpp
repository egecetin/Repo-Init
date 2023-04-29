#pragma once

#include <linux/if_packet.h>

#include <string>

/**
 * @brief Stats produced by RawSocket
 */
struct RawSocketStats
{
	/// Number of bytes written to socket
	size_t sentBytes;
	/// Number of bytes read from socket
	size_t receivedBytes;
	/// Total execution time in nanoseconds
	double processingTime;
};

/**
 * @brief Raw socket reads and writes binary data to provided interface. Write operations does not modify any field
 * (MAC, IP etc). Only writes the full data directly like file write operations.
 */
class RawSocket
{
  private:
	/// Ready flag
	bool isReady;
	/// Mode indicator. True = Write, False = Read
	bool writeMode;
	/// Socket descriptor
	int sockFd;
	/// Currently used ethernet interface
	std::string iFace;
	/// Socket structure
	struct sockaddr_ll addr;
	/// Internal structure for statistics
	RawSocketStats stats;

  public:
	/**
	 * @brief Construct a new RawSocket class
	 * @param[in] iface Ethernet interface
	 * @param[in] isWrite True if write mode, false if read mode requested
	 */
	explicit RawSocket(const std::string &iface, bool isWrite = false);

	/**
	 * @brief Returns the binded ethernet interface
	 * @return std::string Name of the interface
	 */
	std::string getInterfaceName() { return iFace; }

	/**
	 * @brief Writes data to interface
	 * @param[in] data Full payload data to write
	 * @param[in] dataLen Length of the data
	 * @return int Status of operation. Return the number of written bytes, negative on errors.
	 */
	int writeData(const void *data, size_t dataLen);

	/**
	 * @brief Reads data from interface
	 * @param[out] data User allocated data
	 * @param[out] dataLen Length of the data
	 * @return int Status of operation. Return the number of read bytes, negative on errors.
	 */
	int readData(void *data, size_t dataLen);

	/**
	 * @brief Get the statistics of class
	 * @param[in] resetInternalStats Whether internal statistics structure should be reseted after returned
	 * @return RawSocketStats Produced statistics
	 */
	RawSocketStats getStats(bool resetInternalStats = false);

	/**
	 * @brief Destroy the RawSocket object
	 */
	~RawSocket();
};
