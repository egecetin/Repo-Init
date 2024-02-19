#pragma once

#include <zmq.hpp>

/**
 * @class ZeroMQ
 * @brief A class that provides a wrapper for ZeroMQ functionality.
 *
 * This class encapsulates the ZeroMQ library and provides methods to initialize, start, stop, send, and receive
 * messages using ZeroMQ sockets. It supports both binding and connecting to sockets and provides a convenient interface
 * for working with multipart messages.
 */
class ZeroMQ {
  private:
	// Internal context
	std::shared_ptr<zmq::context_t> contextPtr;
	// Internal socket
	std::unique_ptr<zmq::socket_t> socketPtr;

	// Is currently active
	bool isActive{false};
	// Should be binded
	bool isBinded{false};
	// Address to bind/connect
	std::string socketAddr;

	// Initializes class
	void init(const std::shared_ptr<zmq::context_t> &ctx, const zmq::socket_type &type, const std::string &addr,
			  bool isBind);

  public:
	/**
	 * @brief Construct a new ZeroMQ class
	 * @param[in] type Type of the socket
	 * @param[in] addr Full socket address
	 * @param[in] isBind True if should be binded, false if should be connected
	 */
	ZeroMQ(const zmq::socket_type &type, const std::string &addr, bool isBind);

	/**
	 * @brief Construct a new ZeroMQ class
	 * @param[in] ctx ZeroMQ context
	 * @param[in] type Type of the socket
	 * @param[in] addr Full socket address
	 * @param[in] isBind True if should be binded, false if should be connected
	 */
	ZeroMQ(const std::shared_ptr<zmq::context_t> &ctx, const zmq::socket_type &type, const std::string &addr,
		   bool isBind);

	/// @brief Copy constructor
	ZeroMQ(const ZeroMQ & /*unused*/) = delete;

	/// @brief Move constructor
	ZeroMQ(ZeroMQ && /*unused*/) = delete;

	/// @brief Copy assignment operator
	ZeroMQ &operator=(ZeroMQ /*unused*/) = delete;

	/// @brief Move assignment operator
	ZeroMQ &operator=(ZeroMQ && /*unused*/) = delete;

	/**
	 * @brief Starts the connection
	 * @return True if successfully initialized
	 */
	bool start();

	/**
	 * @brief Stops the connection
	 */
	void stop();

	/**
	 * @brief Receives multipart message
	 * @return std::vector<zmq::message_t> Received messages
	 */
	std::vector<zmq::message_t> recvMessages();

	/**
	 * @brief Sends multipart message
	 * @param[in] msg Messages to send
	 * @return size_t Number of sent messages
	 */
	size_t sendMessages(std::vector<zmq::message_t> &msg);

	/**
	 * @brief Get the reference of socket
	 * @return const std::unique_ptr<zmq::socket_t>&
	 */
	const std::unique_ptr<zmq::socket_t> &getSocket() { return socketPtr; }

	/**
	 * @brief Get the reference of context
	 * @return const std::shared_ptr<zmq::context_t>&
	 */
	const std::shared_ptr<zmq::context_t> &getContext() { return contextPtr; }

	/**
	 * @brief Destroy the ZeroMQ class
	 */
	~ZeroMQ();
};
