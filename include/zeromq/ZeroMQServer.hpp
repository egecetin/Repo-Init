#pragma once

#include "connection/Zeromq.hpp"
#include "zeromq/ZeroMQStats.hpp"

#include <functional>

using FPTR_MessageCallback = std::function<bool(const std::vector<zmq::message_t> &, std::vector<zmq::message_t> &)>;

class ZeroMQServer : private ZeroMQ {
  private:
	// Statistics
	std::unique_ptr<ZeroMQStats> stats;
	// Called after every message function(std::vector<zmq::message_t>) {}
	FPTR_MessageCallback m_messageCallback;

  public:
	/**
	 * @brief Constructor for server
	 * @param[in] hostAddr Host address to connect. Can be anything supported by ZeroMQ reply socket
	 * @param[in] reg Prometheus registry for stats
	 */
	explicit ZeroMQServer(const std::string &hostAddr, const std::shared_ptr<prometheus::Registry> &reg = nullptr);

	/**
	 * @brief Initializes a new ZeroMQ server
	 * @return true If initialized
	 * @return false otherwise
	 */
	bool initialise();

	/// Processes new messages
	void update();

	/// Closes the ZeroMQ Server
	void shutdown();

	/**
	 * @brief Sets the message callback function
	 * @param[in] func The message callback function to be set
	 */
	void messageCallback(FPTR_MessageCallback func) { m_messageCallback = std::move(func); }

	/**
	 * @brief Gets the message callback function
	 * @return The message callback function
	 */
	FPTR_MessageCallback messageCallback() const { return m_messageCallback; }
};

/**
 * @brief ZeroMQ message received callback
 * @param[in] recvMsgs Received messages
 * @param[out] replyMsgs Reply messages returned by callback
 * @return true If the callback successfully processes the received messages
 * @return false otherwise
 */
bool ZeroMQServerMessageCallback(const std::vector<zmq::message_t> &recvMsgs, std::vector<zmq::message_t> &replyMsgs);
