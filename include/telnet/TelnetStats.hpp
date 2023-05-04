#pragma once

#include <prometheus/registry.h>

/**
 * @brief Telnet session statistics
 */
struct TelnetSessionStats
{
	/// Connection start time
	std::chrono::high_resolution_clock::time_point connectTime;
	/// Connection end time
	std::chrono::high_resolution_clock::time_point disconnectTime;
	/// Uploaded bytes
	size_t uploadBytes{};
	/// Downloaded bytes
	size_t downloadBytes{};
	/// Successful commands
	uint64_t successCmdCtr{};
	/// Failed commands
	uint64_t failCmdCtr{};
};

/**
 * @brief Telnet server statistics
 */
struct TelnetServerStats
{
	/// Processing time start
	std::chrono::high_resolution_clock::time_point processingTimeStart;
	/// Processing time end
	std::chrono::high_resolution_clock::time_point processingTimeEnd;
	/// Number of active connections
	uint64_t activeConnectionCtr{0};
	/// Number of accepted connections
	uint64_t acceptedConnectionCtr{0};
	/// Number of refused connections
	uint64_t refusedConnectionCtr{0};
};

/**
 * @brief Prometheus statistics for Telnet server
 */
class TelnetStats
{
  private:
	// Information metric family
	prometheus::Family<prometheus::Info> *infoFamily;

	// Number of active connections
	prometheus::Gauge *activeConnection;
	// Number of refused connections
	prometheus::Counter *refusedConnection;
	// Number of total received connections
	prometheus::Counter *totalConnection;

	// Value of the command processing performance
	prometheus::Summary *processingTime;
	// Maximum value of the command processing performance
	prometheus::Gauge *maxProcessingTime;
	// Minimum value of the command processing performance
	prometheus::Gauge *minProcessingTime;

	// Number of succeeded commands
	prometheus::Counter *succeededCommand;
	// Number of failed commands
	prometheus::Counter *failedCommand;
	// Number of total received commands
	prometheus::Counter *totalCommand;

	// Total uploaded bytes
	prometheus::Counter *totalUploadBytes;
	// Total downloaded bytes
	prometheus::Counter *totalDownloadBytes;

	// Value of the duration of sessions
	prometheus::Summary *sessionDuration;
	// Maximum duration of sessions
	prometheus::Gauge *maxSessionDuration;
	// Minimum duration of sessions
	prometheus::Gauge *minSessionDuration;

  public:
	/**
	 * @brief Construct a new Telnet statistics
	 * @param[in] reg Prometheus registry
	 * @param[in] portNumber Telnet server port
	 */
	TelnetStats(const std::shared_ptr<prometheus::Registry> &reg, uint16_t portNumber);

	/**
	 * @brief Updates statistics with session values
	 * @param[in] stat Statistics values from session
	 * @param[in] sessionClosed True if the provided session is closed
	 */
	void consumeStats(TelnetSessionStats &stat, bool sessionClosed);

	/**
	 * @brief Updates statistics with server values
	 * @param[in] stat Statistics values from server
	 */
	void consumeStats(TelnetServerStats &stat);
};
