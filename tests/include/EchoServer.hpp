#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

/**
 * @class EchoServer
 * A simple HTTP echo server for testing purposes
 * Listens on a TCP socket and echoes back the received HTTP request
 */
class EchoServer {
  private:
	int _serverSocket{-1};
	std::jthread _serverThread;

	/**
	 * Server loop that handles incoming connections
	 * @param[in] stopToken Stop token for cooperative cancellation
	 */
	void serverLoop(std::stop_token stopToken)
	{
		while (!stopToken.stop_requested())
		{
			// Accept incoming connection
			sockaddr_in clientAddr{};
			socklen_t clientAddrLen = sizeof(clientAddr);
			int clientSocket = accept(_serverSocket, reinterpret_cast<sockaddr *>(&clientAddr), &clientAddrLen);

			if (clientSocket < 0)
			{
				if (!stopToken.stop_requested())
				{
					continue;
				}
				break;
			}

			// Read the HTTP request
			char buffer[4096] = {0};
			ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

			if (bytesRead > 0)
			{
				// Use explicit length constructor to ensure exact byte count
				std::string request(buffer, bytesRead);

				// Extract only the body from the HTTP request
				// HTTP headers end with "\r\n\r\n", body starts after that
				std::string body;
				size_t bodyStart = request.find("\r\n\r\n");
				if (bodyStart != std::string::npos)
				{
					body = request.substr(bodyStart + 4); // +4 to skip "\r\n\r\n"
				}

				// Build HTTP response with echoed body content
				std::ostringstream response;
				response << "HTTP/1.1 200 OK\r\n";
				response << "Content-Type: text/plain\r\n";
				response << "Content-Length: " << body.length() << "\r\n";
				response << "Connection: close\r\n";
				response << "\r\n";
				response << body;

				// Send response
				std::string responseStr = response.str();
				send(clientSocket, responseStr.c_str(), responseStr.length(), 0);
			}

			close(clientSocket);
		}
	}

  public:
	/**
	 * Constructs a new EchoServer object and starts listening
	 * @param[in] port The port to listen on
	 * @throws std::runtime_error if server fails to start
	 */
	explicit EchoServer(int port)
	{
		_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (_serverSocket < 0)
		{
			throw std::runtime_error("Failed to create socket");
		}

		int opt = 1;
		if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		{
			close(_serverSocket);
			throw std::runtime_error("Failed to set socket options");
		}

		sockaddr_in serverAddr{};
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = INADDR_ANY;
		serverAddr.sin_port = htons(port);

		if (bind(_serverSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) < 0)
		{
			close(_serverSocket);
			throw std::runtime_error("Failed to bind socket to port");
		}

		if (listen(_serverSocket, 5) < 0)
		{
			close(_serverSocket);
			throw std::runtime_error("Failed to listen on socket");
		}

		// Start server thread
		_serverThread = std::jthread(&EchoServer::serverLoop, this);
	}

	/// Destructor - stops the server and cleans up
	~EchoServer()
	{
		_serverThread.request_stop();
		if (_serverSocket >= 0)
		{
			shutdown(_serverSocket, SHUT_RDWR);
			close(_serverSocket);
			_serverSocket = -1;
		}
	}

	/// Deleted copy constructor
	EchoServer(const EchoServer &) = delete;

	/// Deleted copy assignment operator
	EchoServer &operator=(const EchoServer &) = delete;

	/// Deleted move constructor
	EchoServer(EchoServer &&) = delete;

	/// Deleted move assignment operator
	EchoServer &operator=(EchoServer &&) = delete;
};
