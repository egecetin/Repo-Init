#pragma once

#include "connection/Zeromq.hpp"
#include "zeromq/ZeroMQStats.hpp"

#include <functional>

typedef std::function<bool(const std::vector<zmq::message_t> &, std::vector<zmq::const_buffer> &)> FPTR_MessageCallback;

class ZeroMQServer
{
  private:
	// Connection handling pointer
	std::unique_ptr<ZeroMQ> connectionPtr;
	// Connected address
	std::string serverAddr;
	//
	bool m_initialised;
	// Statistics
	std::unique_ptr<ZeroMQStats> stats;

  protected:
	// Called after every message function(std::vector<zmq::message_t>) {}
	FPTR_MessageCallback m_messageCallback;

  public:
	/// Constructor for server
	ZeroMQServer() : m_initialised(false) {}

	/**
	 * @brief Initializes a new ZeroMQ server
	 * @param[in] hostAddr Host address to connect. Can be anything supported by ZeroMQ reply socket
	 * @param[in] reg Prometheus registry for stats
	 * @return true If initialized
	 * @return false otherwise
	 */
	bool initialise(const std::string &hostAddr, std::shared_ptr<prometheus::Registry> reg = nullptr);

	/// Processes new messages
	void update();

	/// Closes the ZeroMQ Server
	void shutdown();

	void messageCallback(FPTR_MessageCallback f) { m_messageCallback = f; }
	FPTR_MessageCallback messageCallback() const { return m_messageCallback; }
};

/**
 * @brief ZeroMQ message received callback
 * @param[in] recvMsgs Received messages
 * @param[out] replyMsgs Reply messages returned by callback
 */
bool ZeroMQServerMessageCallback(const std::vector<zmq::message_t> &recvMsgs,
								 std::vector<zmq::const_buffer> &replyMsgs);
