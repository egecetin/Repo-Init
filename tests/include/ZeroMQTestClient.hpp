#pragma once

#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * Helper to create a message from raw data
 */
template <typename T> zmq::message_t createMessage(const T &data) { return zmq::message_t(&data, sizeof(T)); }

/**
 * Helper to create a message from a string
 */
zmq::message_t createMessage(const std::string &data) { return zmq::message_t(data.data(), data.size()); }

/**
 * Variadic function to create message vector
 */
template <typename... Args> std::vector<zmq::message_t> makeMessageVector(Args &&...args)
{
	std::vector<zmq::message_t> vec;
	vec.reserve(sizeof...(args));
	(vec.push_back(createMessage(std::forward<Args>(args))), ...);
	return vec;
}

/**
 * @class ZeroMQTestClient
 * A simple ZeroMQ test client that sends predefined test messages
 * Connects to a ZeroMQ server and sends various test command sequences
 */
class ZeroMQTestClient {
  private:
	std::shared_ptr<zmq::context_t> _context;
	std::shared_ptr<zmq::socket_t> _socket;

  public:
	/**
	 * Constructs a new ZeroMQTestClient and connects to the server
	 * @param[in] address The address to connect to (e.g., "tcp://127.0.0.1:8300")
	 * @throws std::runtime_error if connection fails
	 */
	explicit ZeroMQTestClient(const std::string &address)
	{
		_context = std::make_shared<zmq::context_t>(1);
		_socket = std::make_shared<zmq::socket_t>(*_context, zmq::socket_type::req);

		try
		{
			_socket->connect(address);
		}
		catch (const zmq::error_t &e)
		{
			throw std::runtime_error(std::string("Failed to connect ZeroMQ socket: ") + e.what());
		}
	}

	/**
	 * Destructor - cleans up socket
	 */
	~ZeroMQTestClient()
	{
		if (_socket)
		{
			_socket->close();
		}
	}

	// Delete copy constructor and assignment operator
	ZeroMQTestClient(const ZeroMQTestClient &) = delete;
	ZeroMQTestClient &operator=(const ZeroMQTestClient &) = delete;

	/**
	 * Send all test messages
	 */
	void sendTestMessages(std::vector<std::vector<zmq::message_t>> &messageArray)
	{
		for (auto &msgs : messageArray)
		{
			zmq::send_multipart(*_socket, msgs);
			std::vector<zmq::message_t> recvMsgs;
			(void)zmq::recv_multipart(*_socket, std::back_inserter(recvMsgs)).value_or(0);
		}
	}
};
