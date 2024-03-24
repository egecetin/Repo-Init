#pragma once

#include <prometheus/registry.h>

/**
 * Telnet session statistics
 */
struct TelnetSessionStats {
	std::chrono::high_resolution_clock::time_point connectTime;	   ///< Connection start time
	std::chrono::high_resolution_clock::time_point disconnectTime; ///< Connection end time
	size_t uploadBytes{};										   ///< Uploaded bytes
	size_t downloadBytes{};										   ///< Downloaded bytes
	uint64_t successCmdCtr{};									   ///< Successful commands
	uint64_t failCmdCtr{};										   ///< Failed commands
};

/**
 * Telnet server statistics
 */
struct TelnetServerStats {
	std::chrono::high_resolution_clock::time_point processingTimeStart; ///< Processing time start
	std::chrono::high_resolution_clock::time_point processingTimeEnd;	///< Processing time end
	uint64_t activeConnectionCtr{};										///< Number of active connections
	uint64_t acceptedConnectionCtr{};									///< Number of accepted connections
	uint64_t refusedConnectionCtr{};									///< Number of refused connections
};

/**
 * Prometheus statistics for Telnet server
 */
class TelnetStats {
  private:
	prometheus::Family<prometheus::Info> *infoFamily; ///< Information metric family
	prometheus::Gauge *activeConnection;			  ///< Number of active connections
	prometheus::Counter *refusedConnection;			  ///< Number of refused connections
	prometheus::Counter *totalConnection;			  ///< Number of total received connections
	prometheus::Summary *processingTime;			  ///< Value of the command processing performance
	prometheus::Gauge *maxProcessingTime;			  ///< Maximum value of the command processing performance
	prometheus::Gauge *minProcessingTime;			  ///< Minimum value of the command processing performance
	prometheus::Counter *succeededCommand;			  ///< Number of succeeded commands
	prometheus::Counter *failedCommand;				  ///< Number of failed commands
	prometheus::Counter *totalCommand;				  ///< Number of total received commands
	prometheus::Counter *totalUploadBytes;			  ///< Total uploaded bytes
	prometheus::Counter *totalDownloadBytes;		  ///< Total downloaded bytes
	prometheus::Summary *sessionDuration;			  ///< Value of the duration of sessions
	prometheus::Gauge *maxSessionDuration;			  ///< Maximum duration of sessions
	prometheus::Gauge *minSessionDuration;			  ///< Minimum duration of sessions

  public:
	/**
	 * Construct a new Telnet statistics
	 * @param[in] reg Prometheus registry
	 * @param[in] portNumber Telnet server port
	 */
	TelnetStats(const std::shared_ptr<prometheus::Registry> &reg, uint16_t portNumber);

	/**
	 * Updates statistics with session values
	 * @param[in] stat Statistics values from session
	 * @param[in] sessionClosed True if the provided session is closed
	 */
	void consumeStats(const TelnetSessionStats &stat, bool sessionClosed);

	/**
	 * Updates statistics with server values
	 * @param[in] stat Statistics values from server
	 */
	void consumeStats(const TelnetServerStats &stat);
};
