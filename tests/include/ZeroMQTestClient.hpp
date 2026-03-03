#pragma once

#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * @class ZeroMQTestClient
 * A simple ZeroMQ test client that sends predefined test messages
 * Connects to a ZeroMQ server and sends various test command sequences
 */
class ZeroMQTestClient {
  private:
	std::shared_ptr<zmq::context_t> _context;
	std::shared_ptr<zmq::socket_t> _socket;

	/**
	 * Helper to create a message from raw data
	 */
	template <typename T> zmq::message_t createMessage(const T &data)
	{
		return zmq::message_t(&data, sizeof(T));
	}

	/**
	 * Helper to create a message from a string
	 */
	zmq::message_t createMessage(const std::string &data) { return zmq::message_t(data.data(), data.size()); }

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
	 * Send all test messages matching the Python script behavior
	 * Tests various command types: version, log level, ping, status
	 */
	void sendTestMessages()
	{
		// Define test messages (matching zeromq-test.py)
		std::vector<std::vector<zmq::message_t>> messageArray;

		// Command codes from the Python script
		const uint32_t CMD_VERSION = 1230128470;    // "version" hash
		const uint32_t CMD_LOG_LEVEL = 1279741772;  // "log level" hash
		const uint32_t CMD_PING = 1196312912;       // "ping" hash
		const uint32_t CMD_STATUS = 1263027027;     // "status" hash

		// Ask version (success)
		{
			std::vector<zmq::message_t> msgs;
			msgs.push_back(createMessage(CMD_VERSION));
			messageArray.push_back(std::move(msgs));
		}

		// Ask version (fail - extra data)
		{
			std::vector<zmq::message_t> msgs;
			msgs.push_back(createMessage(CMD_VERSION));
			msgs.push_back(createMessage(std::string("dummy")));
			messageArray.push_back(std::move(msgs));
		}

		// Ask log level (info - "v")
		{
			std::vector<zmq::message_t> msgs;
			msgs.push_back(createMessage(CMD_LOG_LEVEL));
			msgs.push_back(createMessage(std::string("v")));
			messageArray.push_back(std::move(msgs));
		}

		// Ask log level (debug - "vv")
		{
			std::vector<zmq::message_t> msgs;
			msgs.push_back(createMessage(CMD_LOG_LEVEL));
			msgs.push_back(createMessage(std::string("vv")));
			messageArray.push_back(std::move(msgs));
		}

		// Ask log level (trace - "vvv")
		{
			std::vector<zmq::message_t> msgs;
			msgs.push_back(createMessage(CMD_LOG_LEVEL));
			msgs.push_back(createMessage(std::string("vvv")));
			messageArray.push_back(std::move(msgs));
		}

		// Ask log level (fail - extra data)
		{
			std::vector<zmq::message_t> msgs;
			msgs.push_back(createMessage(CMD_LOG_LEVEL));
			msgs.push_back(createMessage(std::string("v")));
			msgs.push_back(createMessage(std::string("dummy")));
			messageArray.push_back(std::move(msgs));
		}

		// Ask ping (success)
		{
			std::vector<zmq::message_t> msgs;
			msgs.push_back(createMessage(CMD_PING));
			messageArray.push_back(std::move(msgs));
		}

		// Ask ping (fail - extra data)
		{
			std::vector<zmq::message_t> msgs;
			msgs.push_back(createMessage(CMD_PING));
			msgs.push_back(createMessage(std::string("dummy")));
			messageArray.push_back(std::move(msgs));
		}

		// Ask status (success)
		{
			std::vector<zmq::message_t> msgs;
			msgs.push_back(createMessage(CMD_STATUS));
			messageArray.push_back(std::move(msgs));
		}

		// Ask status (fail - extra data)
		{
			std::vector<zmq::message_t> msgs;
			msgs.push_back(createMessage(CMD_STATUS));
			msgs.push_back(createMessage(std::string("dummy")));
			messageArray.push_back(std::move(msgs));
		}

		// Send all messages
		for (auto &msgs : messageArray)
		{
			try
			{
				// Send multipart message
				zmq::send_multipart(*_socket, msgs);

				// Receive response
				std::vector<zmq::message_t> recvMsgs;
				zmq::recv_multipart(*_socket, std::back_inserter(recvMsgs));
			}
			catch (const zmq::error_t &)
			{
				// Continue even if communication fails
			}
		}
	}
};
