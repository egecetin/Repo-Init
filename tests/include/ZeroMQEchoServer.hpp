#pragma once

#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

/**
 * @class ZeroMQEchoServer
 * A simple ZeroMQ echo server for testing purposes
 * Binds a REP socket and echoes back received messages
 */
class ZeroMQEchoServer {
  private:
	std::shared_ptr<zmq::context_t> _context;
	std::shared_ptr<zmq::socket_t> _socket;
	std::jthread _serverThread;
	int _messageCount;

	/**
	 * Server loop that echoes messages
	 * @param[in] stopToken Stop token for cooperative cancellation
	 */
	void serverLoop(std::stop_token stopToken)
	{
		for (int i = 0; i < _messageCount && !stopToken.stop_requested(); ++i)
		{
			try
			{
				// Receive multipart message
				std::vector<zmq::message_t> recvMsgs;
				auto result = zmq::recv_multipart(*_socket, std::back_inserter(recvMsgs));

				if (result && !recvMsgs.empty())
				{
					// Echo back the received messages
					zmq::send_multipart(*_socket, recvMsgs);
				}
			}
			catch (const zmq::error_t &)
			{
				// Socket might be closed or interrupted
				break;
			}
		}
	}

  public:
	/**
	 * Constructs a new ZeroMQEchoServer object and starts listening
	 * @param[in] address The address to bind to (e.g., "tcp://127.0.0.1:8001")
	 * @param[in] messageCount Number of messages to echo before stopping (default: 1)
	 * @throws std::runtime_error if server fails to start
	 */
	explicit ZeroMQEchoServer(const std::string &address, int messageCount = 1) : _messageCount(messageCount)
	{
		_context = std::make_shared<zmq::context_t>(1);
		_socket = std::make_shared<zmq::socket_t>(*_context, zmq::socket_type::rep);

		try
		{
			_socket->bind(address);
		}
		catch (const zmq::error_t &e)
		{
			throw std::runtime_error(std::string("Failed to bind ZeroMQ socket: ") + e.what());
		}

		// Start server thread
		_serverThread = std::jthread([this](std::stop_token stopToken) { this->serverLoop(stopToken); });
	}

	/**
	 * Destructor - cleans up socket and stops server thread
	 */
	~ZeroMQEchoServer()
	{
		if (_serverThread.joinable())
		{
			_serverThread.request_stop();
			_socket->close();
			_serverThread.join();
		}
	}

	// Delete copy constructor and assignment operator
	ZeroMQEchoServer(const ZeroMQEchoServer &) = delete;
	ZeroMQEchoServer &operator=(const ZeroMQEchoServer &) = delete;

	/**
	 * Wait for the server thread to finish
	 */
	void wait()
	{
		if (_serverThread.joinable())
		{
			_serverThread.join();
		}
	}
};
