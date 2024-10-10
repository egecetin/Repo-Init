#pragma once

#include "utils/Hasher.hpp"
#include "zeromq/ZeroMQ.hpp"

#include <thread>

class AuthPermissionChecker {
  public:
	/**
	 * Mechanism for authentication
	 */
	enum Mechanism
	{
		/// NULL mechanism
		NULLMECH = constHasher("NULL"),
		PLAIN = constHasher("PLAIN"),
		CURVE = constHasher("CURVE")
	};

  protected:
	/**
	 * Construct a new AuthPermissionChecker object
	 */
	AuthPermissionChecker() = default;

	/**
	 * Check if the domain is allowed
	 * @param[in] domain Domain to check
	 * @return true If allowed
	 * @return false otherwise
	 */
	bool checkDomainAllowed(const std::string &domain);

	/**
	 * Check if the address is allowed
	 * @param[in] address Address to check
	 * @return true If allowed
	 * @return false otherwise
	 */
	bool checkAddressAllowed(const std::string &address);

	/**
	 * Check if the identity is allowed
	 * @param[in] identity Identity to check
	 * @return true If allowed
	 * @return false otherwise
	 */
	bool checkIdentityAllowed(const std::string &identity);

	/**
	 * Check if the mechanism is allowed
	 * @param[in] mechanism Mechanism to check
	 * @return true If allowed
	 * @return false otherwise
	 */
	bool checkMechanismAllowed(const std::string &mechanism);

	/**
	 * Authenticate credentials
	 * @param[in] recvMsgs Received messages
	 * @param[in] mechanism Mechanism to use
	 * @return true If authenticated
	 * @return false otherwise
	 */
	bool authenticateCredentials(const std::vector<zmq::message_t> &recvMsgs, Mechanism mechanism);

	/**
	 * Deconstructor for AuthPermissionChecker
	 */
	~AuthPermissionChecker() = default;
};

/**
 * @class ZeroMQAuth
 * A class that provides a wrapper for ZeroMQ authentication functionality.
 *
 * This class encapsulates the ZeroMQ library and provides methods to authenticate new connections using ZeroMQ sockets.
 * It starts a separate thread to listen for authentication requests and invokes corresponding event handlers.
 */
class ZeroMQAuth : private ZeroMQ, private AuthPermissionChecker {
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
