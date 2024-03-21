#pragma once

#include "zeromq/ZeroMQ.hpp"
#include "zeromq/ZeroMQMonitor.hpp"
#include "zeromq/ZeroMQStats.hpp"

#include <functional>

using FPTR_MessageCallback = std::function<bool(const std::vector<zmq::message_t> &, std::vector<zmq::message_t> &)>;

class ZeroMQServer : private ZeroMQ, ZeroMQMonitor {
  private:
	// Thread for processing messages
	std::unique_ptr<std::thread> _serverThread;
	// Flag to stop processing messages
	std::atomic_flag _shouldStop{false};
	// Flag to check if the server is running
	std::shared_ptr<std::atomic_flag> _checkFlag;
	// Statistics
	std::unique_ptr<ZeroMQStats> _stats;
	// Called after every message function(std::vector<zmq::message_t>) {}
	FPTR_MessageCallback _m_messageCallback;

	/// Processes new messages
	void update();

	/// Main thread function
	void threadFunc();

  public:
	/**
	 * @brief Constructor for server
	 * @param[in] hostAddr Host address to connect. Can be anything supported by ZeroMQ reply socket
	 * @param[in] checkFlag Flag to check if the server is running
	 * @param[in] reg Prometheus registry for stats
	 */
	explicit ZeroMQServer(const std::string &hostAddr, const std::shared_ptr<std::atomic_flag> &checkFlag,
						  const std::shared_ptr<prometheus::Registry> &reg = nullptr);

	/**
	 * @brief Initializes a new ZeroMQ server
	 * @return true If initialized
	 * @return false otherwise
	 */
	bool initialise();

	/// Closes the ZeroMQ Server
	void shutdown();

	/**
	 * @brief Deconstructor for server
	 */
	~ZeroMQServer() { shutdown(); }

	/**
	 * @brief Sets the message callback function
	 * @param[in] func The message callback function to be set
	 */
	void messageCallback(FPTR_MessageCallback func) { _m_messageCallback = std::move(func); }

	/**
	 * @brief Gets the message callback function
	 * @return The message callback function
	 */
	FPTR_MessageCallback messageCallback() const { return _m_messageCallback; }
};

/**
 * @brief ZeroMQ message received callback
 * @param[in] recvMsgs Received messages
 * @param[out] replyMsgs Reply messages returned by callback
 * @return true If the callback successfully processes the received messages
 * @return false otherwise
 */
bool ZeroMQServerMessageCallback(const std::vector<zmq::message_t> &recvMsgs, std::vector<zmq::message_t> &replyMsgs);
