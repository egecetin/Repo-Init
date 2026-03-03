#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

/**
 * @class TelnetClient
 * A simple telnet client for testing purposes
 * Connects to a telnet server and sends commands
 */
class TelnetClient {
  private:
	int _sockfd{-1};
	std::string _host;
	int _port;
	std::jthread _clientThread;

	/**
	 * Client loop that sends commands
	 * @param[in] commands List of commands to send
	 * @param[in] delayMs Delay between commands in milliseconds
	 * @param[in] stopToken Stop token for cooperative cancellation
	 */
	void clientLoop(std::vector<std::string> commands, std::stop_token stopToken)
	{
		for (const auto &command : commands)
		{
			if (stopToken.stop_requested())
			{
				break;
			}

			ssize_t sent = send(_sockfd, command.c_str(), command.length(), 0);
			if (sent < 0)
			{
				break;
			}

			// Read response (discard for testing purposes)
			char buffer[4096];
			recv(_sockfd, buffer, sizeof(buffer), MSG_DONTWAIT);
		}
	}

  public:
	/**
	 * Constructs a new TelnetClient object and connects to the server
	 * @param[in] host The host to connect to (e.g., "localhost")
	 * @param[in] port The port to connect to
	 * @param[in] commands List of commands to send (if empty, just connects)
	 * @throws std::runtime_error if connection fails
	 */
	explicit TelnetClient(const std::string &host, int port, const std::vector<std::string> &commands = {})
		: _host(host), _port(port)
	{
		// Create socket
		_sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (_sockfd < 0)
		{
			throw std::runtime_error("Failed to create socket");
		}

		// Set up server address
		struct sockaddr_in serverAddr;
		memset(&serverAddr, 0, sizeof(serverAddr));
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(port);

		if (inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr) <= 0)
		{
			close(_sockfd);
			throw std::runtime_error("Invalid address");
		}

		// Connect to server
		if (connect(_sockfd, reinterpret_cast<struct sockaddr *>(&serverAddr), sizeof(serverAddr)) < 0)
		{
			close(_sockfd);
			throw std::runtime_error("Connection failed");
		}

		// If commands provided, start sending them in a thread
		if (!commands.empty())
		{
			_clientThread = std::jthread([this, commands](std::stop_token stopToken) {
				this->clientLoop(commands, stopToken);
			});
		}
	}

	/**
	 * Destructor - cleans up socket and stops client thread
	 */
	~TelnetClient()
	{
		if (_clientThread.joinable())
		{
			_clientThread.request_stop();
			_clientThread.join();
		}

		if (_sockfd >= 0)
		{
			close(_sockfd);
		}
	}

	// Delete copy constructor and assignment operator
	TelnetClient(const TelnetClient &) = delete;
	TelnetClient &operator=(const TelnetClient &) = delete;

	/**
	 * Send a single command immediately
	 * @param[in] command The command to send
	 * @return true if sent successfully, false otherwise
	 */
	bool sendCommand(const std::string &command)
	{
		if (_sockfd < 0)
		{
			return false;
		}

		ssize_t sent = send(_sockfd, command.c_str(), command.length(), 0);
		return sent > 0;
	}

	/**
	 * Wait for the client thread to finish (if running)
	 */
	void wait()
	{
		if (_clientThread.joinable())
		{
			_clientThread.join();
		}
	}
};
