#pragma once

#include <prometheus/registry.h>
#include <zmq.hpp>

/**
 * @struct ZeroMQServerStats
 * @brief Represents the statistics of a ZeroMQ server connection.
 */
struct ZeroMQServerStats {
	std::chrono::high_resolution_clock::time_point processingTimeStart; ///< Processing time start
	std::chrono::high_resolution_clock::time_point processingTimeEnd;	///< Processing time end
	bool isSuccessful{false}; ///< Indicates if processing was successful for this connection
};

/**
 * @class ZeroMQStats
 * @brief Represents the statistics of a ZeroMQ server.
 */
class ZeroMQStats {
  private:
	prometheus::Family<prometheus::Info> *infoFamily; ///< Information metric family
	prometheus::Summary *processingTime;			  ///< Value of the command processing performance
	prometheus::Gauge *maxProcessingTime;			  ///< Maximum value of the command processing performance
	prometheus::Gauge *minProcessingTime;			  ///< Minimum value of the command processing performance
	prometheus::Counter *succeededCommand;			  ///< Number of succeeded commands
	prometheus::Counter *failedCommand;				  ///< Number of failed commands
	prometheus::Counter *totalCommand;				  ///< Number of total received commands
	prometheus::Counter *succeededCommandParts;		  ///< Number of received succeeded message parts
	prometheus::Counter *failedCommandParts;		  ///< Number of received failed message parts
	prometheus::Counter *totalCommandParts;			  ///< Number of received total message parts
	prometheus::Counter *totalUploadBytes;			  ///< Total uploaded bytes
	prometheus::Counter *totalDownloadBytes;		  ///< Total downloaded bytes

  public:
	/**
	 * @brief Construct a new ZeroMQStats object.
	 * @param[in] reg Prometheus registry.
	 */
	explicit ZeroMQStats(const std::shared_ptr<prometheus::Registry> &reg);

	/**
	 * @brief Updates the statistics with messages.
	 * @param[in] recvMsgs Received messages.
	 * @param[in] sendMsgs Send messages.
	 * @param[in] serverStats ZeroMQ server stats.
	 */
	void consumeStats(const std::vector<zmq::message_t> &recvMsgs, const std::vector<zmq::message_t> &sendMsgs,
					  const ZeroMQServerStats &serverStats);
};
