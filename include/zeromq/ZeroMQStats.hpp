#pragma once

#include <prometheus/registry.h>
#include <zmq.hpp>

struct ZeroMQServerStats
{
	/// Processing time start
	std::chrono::high_resolution_clock::time_point processingTimeStart;
	/// Processing time end
	std::chrono::high_resolution_clock::time_point processingTimeEnd;
	/// Is processing successful for this connection
	bool isSuccessful;
};

class ZeroMQStats
{
  private:
	// Information metric family
	prometheus::Family<prometheus::Info> *infoFamily;

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
	// Number of received succeeded message parts
	prometheus::Counter *succeededCommandParts;
	// Number of received failed message parts
	prometheus::Counter *failedCommandParts;
	// Number of received total message parts
	prometheus::Counter *totalCommandParts;

	// Total uploaded bytes
	prometheus::Counter *totalUploadBytes;
	// Total downloaded bytes
	prometheus::Counter *totalDownloadBytes;

  public:
	/**
	 * @brief Construct a new ZeroMQ server statistics
	 * @param[in] reg Prometheus registry
	 */
	explicit ZeroMQStats(std::shared_ptr<prometheus::Registry> reg);

	/**
	 * @brief Updates statistics with messages
	 * @param[in] recvMsgs Received messages
	 * @param[in] sendMsgs Send messages
	 * @param[in] serverStats ZeroMQ server stats
	 */
	void consumeStats(const std::vector<zmq::message_t> &recvMsgs, const std::vector<zmq::const_buffer> &sendMsgs,
					  const ZeroMQServerStats &serverStats);
};
