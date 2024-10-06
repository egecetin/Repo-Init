#pragma once

#include "zeromq/ZeroMQ.hpp"
#include "utils/Hasher.hpp"

#include <thread>

/**
 * @class ZeroMQAuth
 * A class that provides a wrapper for ZeroMQ authentication functionality.
 * 
 * This class encapsulates the ZeroMQ library and provides methods to authenticate new connections using ZeroMQ sockets.
 * It starts a separate thread to listen for authentication requests and invokes corresponding event handlers.
 */
class ZeroMQAuth : private ZeroMQ {
  private:
	// Thread for processing messages
	std::unique_ptr<std::thread> _authThread;
	// Flag to stop processing messages
	std::atomic_flag _shouldStop{false};
	// Statistics
	std::unique_ptr<ZeroMQStats> _stats;

	/// Authenticate new connections
	bool authenticateConnection(const std::vector<zmq::message_t> &recvMsgs, std::vector<zmq::message_t> &replyMsgs);
	/// Processes new messages
	void update();
	/// Main thread function
	void threadFunc() noexcept;

  protected:
	/**
	 * Start authentication thread
	 * @return true If initialized
	 * @return false otherwise
	 */
	bool startAuth();

	/**
	 * Stop authentication thread
	 */
	void stopAuth();

  public:

    /**
     * Mechanism for authentication
     */
    enum Mechanism {
        /// NULL mechanism
        NULLMECH = constHasher("NULL"),
        PLAIN = constHasher("PLAIN"),
        CURVE = constHasher("CURVE")
    };

	/**
	 * Construct a new ZeroMQAuth class
	 * @param[in] ctx ZeroMQ context
	 * @param[in] reg Prometheus registry for stats
	 * @param[in] prependName Prefix for Prometheus stats
	 */
	ZeroMQAuth(const std::shared_ptr<zmq::context_t> &ctx, const std::shared_ptr<prometheus::Registry> &reg = nullptr,
			   const std::string &prependName = "");

	/**
	 * Deconstructor for ZeroMQAuth
	 */
	~ZeroMQAuth() { stopAuth(); }
};
