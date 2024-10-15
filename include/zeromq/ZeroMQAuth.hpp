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

/**
 * Permission checker
 * This class is used to check if a entity is allowed or disallowed
 * It can be used to check if a domain or identity is allowed or disallowed
 */
class PermissionChecker {
  private:
	/// Whether unknown entities are allowed
	bool _isUnknownAllowed{false};
	/// Allowed entities
	std::unordered_set<std::string> _allowed;
	/// Disallowed entities
	std::unordered_set<std::string> _disallowed;

	/// Modify allowed/disallowed filters
	inline bool modifyEntry(std::unordered_set<std::string> &list, const std::string &entry, bool add)
	{
		return add ? list.insert(entry).second : list.erase(entry);
	}

	template <typename IterableContainer>
	inline bool modifyEntry(std::unordered_set<std::string> &list, const IterableContainer &entries, bool add);

  public:
	/**
	 * Check if the entry is allowed
	 * @param[in] entry Entry to check
	 * @return true If allowed
	 * @return false otherwise
	 */
	bool checkAllowed(const std::string &entry)
	{
		return _isUnknownAllowed &&
			   ((_allowed.find(entry) == _allowed.end()) || (_disallowed.find(entry) == _disallowed.end()));
	}

	/**
	 * Whether unknown entries should be allowed or not
	 * @param[in] allow True if should allow
	 */
	void allowUnknown(bool allow) { _isUnknownAllowed = allow; }

	/**
	 * Is allowing unknown entries
	 * @return true If allowed
	 * @return false otherwise
	 */
	bool isUnknownAllowed() const { return _isUnknownAllowed; }

	/**
	 * Allow entry
	 * @param[in] entry Entry to allow
	 */
	bool allow(const std::string &entry) { return modifyEntry(_allowed, entry, true); }

	/**
	 * Allow multiple entries
	 * @param[in] entries Entries to allow
	 * @tparam IterableContainer Type of the container. Must be iterable and contain std::string elements.
	 * @return true If all entries were allowed
	 */
	template <typename IterableContainer> bool allow(const IterableContainer &entries)
	{
		return modifyEntry(_allowed, entries, true);
	}

	/**
	 * Remove allowed entry
	 * @param[in] entry Entry to remove
	 */
	bool removeAllowed(const std::string &entry) { return modifyEntry(_allowed, entry, false); }

	/**
	 * Remove multiple allowed entries
	 * @param[in] entries Entries to remove
	 * @tparam IterableContainer Type of the container. Must be iterable and contain std::string elements.
	 * @return true If all entries were removed
	 */
	template <typename IterableContainer> bool removeAllowed(const IterableContainer &entries)
	{
		return modifyEntry(_allowed, entries, false);
	}

	/**
	 * Disallow entry
	 * @param[in] entry Entry to disallow
	 */
	bool disallow(const std::string &entry) { return modifyEntry(_disallowed, entry, true); }

	/**
	 * Disallow multiple entries
	 * @param[in] entries Entries to disallow
	 * @tparam IterableContainer Type of the container. Must be iterable and contain std::string elements.
	 * @return true If all entries were disallowed
	 */
	template <typename IterableContainer> bool disallow(const IterableContainer &entries)
	{
		return modifyEntry(_disallowed, entries, true);
	}

	/**
	 * Remove disallowed entry
	 * @param[in] entry Entry to remove
	 */
	bool removeDisallowed(const std::string &entry) { return modifyEntry(_disallowed, entry, false); }

	/**
	 * Remove multiple disallowed entries
	 * @param[in] entries Entries to remove
	 * @tparam IterableContainer Type of the container. Must be iterable and contain std::string elements.
	 * @return true If all entries were removed
	 */
	template <typename IterableContainer> bool removeDisallowed(const IterableContainer &entries)
	{
		return modifyEntry(_disallowed, entries, false);
	}
};

/**
 * Class to authenticate ZeroMQ connections. It has filter lists to allow or disallow domains, addresses and identities.
 */
class AuthPermissionChecker {
	friend class PermissionChecker;

  private:
	/// Domain filters
	PermissionChecker _domainChecker;
	/// Address filters
	PermissionChecker _addressChecker;
	/// Identity filters
	PermissionChecker _identityChecker;

  protected:
	/**
	 * Construct a new AuthPermissionChecker object with default values
	 * Initially all unknown entities are disallowed. You should enable them if needed.
	 * @param[in] allowedDomains File path with allowed domains. One domain per line. Empty string if not used.
	 * @param[in] disallowedDomains File path with disallowed domains. One domain per line. Empty string if not used.
	 * @param[in] allowedAddresses File path with allowed addresses. One address per line. Empty string if not used.
	 * @param[in] disallowedAddresses File path with disallowed addresses. One address per line. Empty string if not
	 * used.
	 * @param[in] allowedIdentities File path with allowed identities. One identity per line. Empty string if not used.
	 * @param[in] disallowedIdentities File path with disallowed identities. One identity per line. Empty string if not
	 * used.
	 */
	AuthPermissionChecker(const std::string &allowedDomains = "", const std::string &disallowedDomains = "",
						   const std::string &allowedAddresses = "", const std::string &disallowedAddresses = "",
						   const std::string &allowedIdentities = "", const std::string &disallowedIdentities = "");

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

  public:
	/**
	 * Should allow unknown domains
	 * @param[in] allow True if should allow
	 */
	void shouldAllowUnknownDomain(bool allow) { _domainChecker.allowUnknown(allow); }

	/**
	 * Is allowing unknown domains
	 * @return true If allowed
	 * @return false otherwise
	 */
	bool isUnknownDomainAllowed() const { return _domainChecker.isUnknownAllowed(); }

	/**
	 * Should allow unknown addresses
	 * @param[in] allow True if should allow
	 */
	void shouldAllowUnknownAddress(bool allow) { _addressChecker.allowUnknown(allow); }

	/**
	 * Is allowing unknown addresses
	 * @return true If allowed
	 * @return false otherwise
	 */
	bool isUnknownAddressAllowed() const { return _addressChecker.isUnknownAllowed(); }

	/**
	 * Should allow unknown identities
	 * @param[in] allow True if should allow
	 */
	void shouldAllowUnknownIdentity(bool allow) { _identityChecker.allowUnknown(allow); }

	/**
	 * Is allowing unknown identities
	 * @return true If allowed
	 * @return false otherwise
	 */
	bool isUnknownIdentityAllowed() const { return _identityChecker.isUnknownAllowed(); }

	/**
	 * Read configuration from text files
	 * @param[in] allowedDomains File path with allowed domains. One domain per line. Empty string if not used.
	 * @param[in] disallowedDomains File path with disallowed domains. One domain per line. Empty string if not used.
	 * @param[in] allowedAddresses File path with allowed addresses. One address per line. Empty string if not used.
	 * @param[in] disallowedAddresses File path with disallowed addresses. One address per line. Empty string if not
	 * used.
	 * @param[in] allowedIdentities File path with allowed identities. One identity per line. Empty string if not used.
	 * @param[in] disallowedIdentities File path with disallowed identities. One identity per line. Empty string if not
	 * used.
	 */
	void readConfiguration(const std::string &allowedDomains = "", const std::string &disallowedDomains = "",
						   const std::string &allowedAddresses = "", const std::string &disallowedAddresses = "",
						   const std::string &allowedIdentities = "", const std::string &disallowedIdentities = "");

	/**
	 * Dump configuration to files
	 * @param[in] allowedDomains File path to dump allowed domains. Empty string if not used.
	 * @param[in] disallowedDomains File path to dump disallowed domains. Empty string if not used.
	 * @param[in] allowedAddresses File path to dump allowed addresses. Empty string if not used.
	 * @param[in] disallowedAddresses File path to dump disallowed addresses. Empty string if not used.
	 * @param[in] allowedIdentities File path to dump allowed identities. Empty string if not used.
	 * @param[in] disallowedIdentities File path to dump disallowed identities. Empty string if not used.
	 */
	void dumpConfiguration(const std::string &allowedDomains = "", const std::string &disallowedDomains = "",
						   const std::string &allowedAddresses = "", const std::string &disallowedAddresses = "",
						   const std::string &allowedIdentities = "",
						   const std::string &disallowedIdentities = "") const;
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

	/// Mechanism filters
	uint8_t _allowedMechanisms{0};

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
