#pragma once

#include "utils/Hasher.hpp"
#include "zeromq/ZeroMQ.hpp"
#include "zeromq/ZeroMQStats.hpp"

#include "Common++/header/IpAddress.h"

#include <thread>
#include <unordered_set>

/**
 * Mechanism for authentication
 */
enum class Mechanism : uint64_t
{
	/// NULL mechanism
	NULLMECH = constHasher("NULL"),
	PLAIN = constHasher("PLAIN"),
	CURVE = constHasher("CURVE")
};

class AuthPermissionChecker {
  private:
	/// Domain filters
	bool _isUnknownDomainAllowed{false};
	std::unordered_set<std::string> _allowedDomains;
	std::unordered_set<std::string> _disallowedDomains;

	/// Address filters
	bool _isUnknownAddressAllowed{false};
	std::vector<pcpp::IPNetwork> _allowedAddresses;
	std::vector<pcpp::IPNetwork> _disallowedAddresses;

	/// Identity filters
	bool _isUnknownIdentityAllowed{false};
	std::unordered_set<std::string> _allowedIdentities;
	std::unordered_set<std::string> _disallowedIdentities;

	/// Mechanism filters
	uint8_t _allowedMechanisms{0};

  protected:
	/**
	 * Construct a new AuthPermissionChecker object with default values
	 * Initially all unknown entities are disallowed. You should enable them if needed.
	 */
	AuthPermissionChecker();

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

  public:
	/**
	 * Should allow unknown domains
	 * @param[in] allow True if should allow
	 */
	void shouldAllowUnknownDomain(bool allow) { _isUnknownDomainAllowed = allow; }

	/**
	 * Is allowing unknown domains
	 * @return true If allowed
	 * @return false otherwise
	 */
	bool isUnknownDomainAllowed() const { return _isUnknownDomainAllowed; }

	/**
	 * Should allow unknown addresses
	 * @param[in] allow True if should allow
	 */
	void shouldAllowUnknownAddress(bool allow) { _isUnknownAddressAllowed = allow; }

	/**
	 * Is allowing unknown addresses
	 * @return true If allowed
	 * @return false otherwise
	 */
	bool isUnknownAddressAllowed() const { return _isUnknownAddressAllowed; }

	/**
	 * Should allow unknown identities
	 * @param[in] allow True if should allow
	 */
	void shouldAllowUnknownIdentity(bool allow) { _isUnknownIdentityAllowed = allow; }

	/**
	 * Is allowing unknown identities
	 * @return true If allowed
	 * @return false otherwise
	 */
	bool isUnknownIdentityAllowed() const { return _isUnknownIdentityAllowed; }
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
