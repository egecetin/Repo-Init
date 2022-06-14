#include "telnet/TelnetServer.h"
#include "Utils.h"

#include <spdlog/spdlog.h>

// Invalid socket identifier for readability
#define INVALID_SOCKET -1
// Receive buffer length
#define DEFAULT_BUFLEN 512
// Timeout to automatic close session
#define TELNET_TIMEOUT 120
// Maximum number of concurrent sessions
#define MAX_AVAILABLE_SESSION 5

std::string TelnetSession::getPeerIP()
{
	struct sockaddr_in client_info;
	memset(&client_info, 0, sizeof(client_info));
	socklen_t addrsize = sizeof(client_info);
	getpeername(m_socket, (struct sockaddr *)&client_info, &addrsize);

	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &client_info.sin_addr, &ip[0], INET_ADDRSTRLEN);

	return ip;
}

void TelnetSession::sendPromptAndBuffer()
{
	// Output the prompt
	int iSendResult =
		send(m_socket, m_telnetServer->promptString().c_str(), m_telnetServer->promptString().length(), 0);

	if (iSendResult < 0)
		spdlog::error("Error on send {}", iSendResult);
	if (m_buffer.length() > 0)
	{
		// resend the buffer
		iSendResult = send(m_socket, m_buffer.c_str(), m_buffer.length(), 0);
		if (iSendResult < 0)
			spdlog::error("Error on send {}", iSendResult);
	}
}

void TelnetSession::eraseLine()
{
	// Send an erase line
	int iSendResult = send(m_socket, ANSI_ERASE_LINE.c_str(), ANSI_ERASE_LINE.length(), 0);
	if (iSendResult < 0)
		spdlog::error("Error on send {}", iSendResult);

	// Move the cursor to the beginning of the line
	std::string moveBack = "\x1b[80D";
	iSendResult = send(m_socket, moveBack.c_str(), moveBack.length(), 0);
	if (iSendResult < 0)
		spdlog::error("Error on send {}", iSendResult);
}

void TelnetSession::sendLine(std::string data)
{
	// If is something is on the prompt, wipe it off
	if (m_telnetServer->interactivePrompt() || m_buffer.length() > 0)
		eraseLine();

	data.append("\r\n");
	int iSendResult = send(m_socket, data.c_str(), data.length(), 0);
	if (iSendResult < 0)
		spdlog::error("Error on send {}", iSendResult);

	if (m_telnetServer->interactivePrompt())
		sendPromptAndBuffer();
}

void TelnetSession::closeClient()
{
	// attempt to cleanly shutdown the connection since we're done
	int iResult = shutdown(m_socket, SHUT_WR);
	if (iResult < 0)
		spdlog::error("Shutdown failed with error {}", strerror(errno));

	// cleanup
	close(m_socket);
}

bool TelnetSession::checkTimeout()
{
	return (currentTime - lastSeenTime > TELNET_TIMEOUT);
}

void TelnetSession::markTimeout()
{
	lastSeenTime = 0;
}

void TelnetSession::echoBack(char *buffer, u_long length)
{
	// If you are an NVT command (i.e. first it of data is 255) then ignore the echo back
	unsigned char firstItem = *buffer;
	if (firstItem == 0xff)
		return;

	int iSendResult = send(m_socket, buffer, length, 0);
	if (iSendResult < 0)
	{
		spdlog::error("Send failed with error {}", strerror(errno));
		close(m_socket);
	}
}

void TelnetSession::initialise()
{
	// Get details of connection
	spdlog::info("Connection received from {}", getPeerIP());

	// Set the connection to be non-blocking
	u_long iMode = 1;
	ioctl(m_socket, FIONBIO, &iMode);

	// Set NVT mode to say that I will echo back characters.
	unsigned char willEcho[3] = {0xff, 0xfb, 0x01};
	int iSendResult = send(m_socket, (char *)willEcho, 3, 0);
	if (iSendResult < 0)
		spdlog::error("Error on send {}", iSendResult);

	// Set NVT requesting that the remote system not/dont echo back characters
	unsigned char dontEcho[3] = {0xff, 0xfe, 0x01};
	iSendResult = send(m_socket, (char *)dontEcho, 3, 0);
	if (iSendResult < 0)
		spdlog::error("Error on send {}", iSendResult);

	// Set NVT mode to say that I will suppress go-ahead. Stops remote clients from doing local linemode.
	unsigned char willSGA[3] = {0xff, 0xfb, 0x03};
	iSendResult = send(m_socket, (char *)willSGA, 3, 0);
	if (iSendResult < 0)
		spdlog::error("Error on send {}", iSendResult);

	if (m_telnetServer->connectedCallback())
		m_telnetServer->connectedCallback()(shared_from_this());

	// Set last seen
	lastSeenTime = currentTime;
}

void TelnetSession::stripNVT(std::string &buffer)
{
	size_t found;
	do
	{
		unsigned char findChar = 0xff;
		found = buffer.find_first_of((char)findChar);
		if (found != std::string::npos && (found + 2) <= buffer.length() - 1)
			buffer.erase(found, 3);
	} while (found != std::string::npos);
}

void TelnetSession::stripEscapeCharacters(std::string &buffer)
{
	size_t found;
	std::array<std::string, 4> cursors = {ANSI_ARROW_UP, ANSI_ARROW_DOWN, ANSI_ARROW_RIGHT, ANSI_ARROW_LEFT};

	for (auto c : cursors)
	{
		do
		{
			found = buffer.find(c);
			if (found != std::string::npos)
				buffer.erase(found, c.length());
		} while (found != std::string::npos);
	}
}

bool TelnetSession::processBackspace(std::string &buffer)
{
	bool foundBackspaces = false;
	size_t found;
	do
	{
		// Need to handle both \x7f and \b backspaces
		unsigned char findChar = '\x7f';
		found = buffer.find_first_of((char)findChar);
		if (found == std::string::npos)
		{
			findChar = '\b';
			found = buffer.find_first_of((char)findChar);
		}

		if (found != std::string::npos)
		{
			if (buffer.length() > 1)
				buffer.erase(found - 1, 2);
			else
				buffer = "";
			foundBackspaces = true;
		}
	} while (found != std::string::npos);
	return foundBackspaces;
}

void TelnetSession::addToHistory(std::string line)
{
	// Add it to the history
	if (line != (m_history.size() > 0 ? m_history.back() : "") && line != "")
	{
		m_history.push_back(line);
		if (m_history.size() > 50)
			m_history.pop_front();
	}
	m_historyCursor = m_history.end();
}

bool TelnetSession::processCommandHistory(std::string &buffer)
{
	// Handle up and down arrow actions
	if (m_telnetServer->interactivePrompt())
	{
		if (buffer.find(ANSI_ARROW_UP) != std::string::npos && m_history.size() > 0)
		{
			if (m_historyCursor != m_history.begin())
				m_historyCursor--;
			buffer = *m_historyCursor;

			// Issue a cursor command to counter it
			int iSendResult = send(m_socket, ANSI_ARROW_DOWN.c_str(), (u_long)ANSI_ARROW_DOWN.length(), 0);
			if (iSendResult < 0)
				spdlog::error("Error on send {}", iSendResult);
			return true;
		}
		if (buffer.find(ANSI_ARROW_DOWN) != std::string::npos && m_history.size() > 0)
		{
			if (next(m_historyCursor) != m_history.end())
				m_historyCursor++;
			buffer = *m_historyCursor;

			// Issue a cursor command to counter it
			int iSendResult = send(m_socket, ANSI_ARROW_UP.c_str(), (u_long)ANSI_ARROW_UP.length(), 0);
			if (iSendResult < 0)
				spdlog::error("Error on send {}", iSendResult);
			return true;
		}
		if (buffer.find(ANSI_ARROW_LEFT) != std::string::npos || buffer.find(ANSI_ARROW_RIGHT) != std::string::npos)
		{
			// Ignore left and right and just reprint buffer
			return true;
		}
	}
	return false;
}

std::vector<std::string> TelnetSession::getCompleteLines(std::string &buffer)
{
	// Now find all new lines (<CR><LF>) and place in a vector and delete from buffer
	std::vector<std::string> lines;
	size_t found;
	do
	{
		found = buffer.find("\r\n");
		if (found != std::string::npos)
		{
			lines.push_back(buffer.substr(0, found));
			buffer.erase(0, found + 2);
		}
	} while (found != std::string::npos);

	return lines;
}

void TelnetSession::update()
{
	int readBytes;
	char recvbuf[DEFAULT_BUFLEN];
	u_long recvbuflen = DEFAULT_BUFLEN;

	readBytes = recv(m_socket, recvbuf, recvbuflen, 0);

	// Check for errors from the read
	if (readBytes < 0 && errno != EAGAIN)
	{
		spdlog::error("Receive failed with error code {}", strerror(errno));
		close(m_socket);
	}
	else if (readBytes > 0)
	{
		// Update last seen
		lastSeenTime = currentTime;

		// Echo it back to the sender
		echoBack(recvbuf, readBytes);

		// we've got to be careful here. Telnet client might send null characters for New Lines mid-data block. We need
		// to swap these out. recv is not null terminated, so its cool
		for (int i = 0; i < readBytes; i++)
		{
			if (recvbuf[i] == 0x00)
				recvbuf[i] = 0x0A; // New Line
		}

		// Add it to the received buffer
		m_buffer.append(recvbuf, readBytes);
		// Remove telnet negotiation sequences
		stripNVT(m_buffer);

		bool requirePromptReprint = false;
		if (m_telnetServer->interactivePrompt())
		{
			if (processCommandHistory(m_buffer)) // Read up and down arrow keys and scroll through history
				requirePromptReprint = true;
			stripEscapeCharacters(m_buffer);

			if (processBackspace(m_buffer))
				requirePromptReprint = true;
		}

		auto lines = getCompleteLines(m_buffer);
		for (auto line : lines)
		{
			if (m_telnetServer->newLineCallBack())
				m_telnetServer->newLineCallBack()(shared_from_this(), line);
			addToHistory(line);
		}

		if (m_telnetServer->interactivePrompt() && requirePromptReprint)
		{
			eraseLine();
			sendPromptAndBuffer();
		}
	}
}

int TelnetSession::UNIT_TEST()
{
	/* stripNVT */
	std::string origData = "12345";
	std::string data = origData;
	unsigned char toStrip[3] = {255, 251, 1};
	data.insert(2, (char *)toStrip, 3);
	TelnetSession::stripNVT(data);

	if (origData != data)
		return -1;

	/* processBackspace */
	std::string bkData = "123455\x7f";
	bool bkResult = TelnetSession::processBackspace(bkData);
	if (bkData != "12345" || !bkResult)
		return -2;

	/* getCompleteLines */
	std::string multiData = "LINE1\r\nLINE2\r\nLINE3\r\n";
	auto lines = TelnetSession::getCompleteLines(multiData);

	if (lines.size() != 3 || (lines[0] != "LINE1") || (lines[1] != "LINE2") || (lines[2] != "LINE3"))
		return -3;
	return 0;
}

/* ------------------ Telnet Server -------------------*/
bool TelnetServer::initialise(u_long listenPort, std::string promptString)
{
	if (m_initialised)
	{
		spdlog::error("This Telnet Server has already been initialised");
		return false;
	}

	m_listenPort = listenPort;
	m_promptString = promptString;
	spdlog::info("Starting Telnet Server on port {}", std::to_string(m_listenPort));

	int iResult;
#ifdef _WIN32
	WSADATA wsaData;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		spdlog::error("WSAStartup failed with error {}", iResult);
		return false;
	}
#endif
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	struct addrinfo *result = NULL;
	iResult = getaddrinfo(NULL, std::to_string(m_listenPort).c_str(), &hints, &result);
	if (iResult != 0)
	{
		spdlog::error("getaddrinfo failed with error {}", iResult);
		return false;
	}

	// Create a SOCKET for connecting to server
	m_listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (m_listenSocket == INVALID_SOCKET)
	{
		spdlog::error("Socket failed with error {}", strerror(errno));
		freeaddrinfo(result);
		return false;
	}

	// Setup the TCP listening socket
	iResult = bind(m_listenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult < 0)
	{
		spdlog::error("Bind failed with error {}", strerror(errno));
		freeaddrinfo(result);
		close(m_listenSocket);
		return false;
	}

	freeaddrinfo(result);

	iResult = listen(m_listenSocket, SOMAXCONN);
	if (iResult < 0)
	{
		spdlog::error("Listen failed with error {}", strerror(errno));
		close(m_listenSocket);
		return false;
	}

	m_initialised = true;
	return true;
}

void TelnetServer::acceptConnection()
{
	Socket ClientSocket = INVALID_SOCKET;
	ClientSocket = accept(m_listenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET)
	{
		spdlog::error("Accept failed with error {}", strerror(errno));
		close(m_listenSocket);
		return;
	}
	else if (m_sessions.size() >= MAX_AVAILABLE_SESSION)
	{
		spdlog::error("Can't accept too many connections {}", m_sessions.size());

		// Create for only sending error
		SP_TelnetSession s = std::make_shared<TelnetSession>(ClientSocket, shared_from_this());
		s->initialise();

		s->sendLine("Too many active connections. Please try again later. \r\nClosing...");
		s->closeClient();
		return;
	}

	SP_TelnetSession s = std::make_shared<TelnetSession>(ClientSocket, shared_from_this());
	m_sessions.push_back(s);
	s->initialise();
}

void TelnetServer::update()
{
	// See if connection pending on the listening socket
	fd_set readSet;
	FD_ZERO(&readSet);
	FD_SET(m_listenSocket, &readSet);
	timeval timeout;
	timeout.tv_sec = 0; // Zero timeout (poll)
	timeout.tv_usec = 0;

	// If there is a connection pending, accept it.
	if (select(m_listenSocket + 1, &readSet, NULL, NULL, &timeout) > 0)
		acceptConnection();

	// Update all the telnet Sessions that are currently in flight.
	for (size_t idx = 0; idx < m_sessions.size(); ++idx)
	{
		m_sessions[idx]->update();
		if (m_sessions[idx]->checkTimeout())
		{
			spdlog::info("Connection closing to {}", m_sessions[idx]->getPeerIP());
			m_sessions[idx]->closeClient();
			m_sessions.erase(m_sessions.begin() + idx);
			--idx;
		}
	}
}

void TelnetServer::shutdown()
{
	// Attempt to cleanly close every telnet session in flight.
	for (SP_TelnetSession ts : m_sessions)
		ts->closeClient();
	m_sessions.clear();

	// No longer need server socket so close it.
	close(m_listenSocket);
	m_initialised = false;
}
