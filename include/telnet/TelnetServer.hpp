/**
Copyright (c) 2015, Luke Malcolm
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
*/

#pragma once

#include "telnet/TelnetStats.hpp"

#include <array>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

class TelnetServer;
class TelnetSession;

typedef int Socket;

/**
 * @brief Session class for manage connections
 */
class TelnetSession : public std::enable_shared_from_this<TelnetSession>
{
  public:
	/// Constructor for session
	TelnetSession(Socket ClientSocket, std::shared_ptr<TelnetServer> ts) : m_socket(ClientSocket), m_telnetServer(ts)
	{
		m_historyCursor = m_history.end();
	};

	/// Send a line of data to the Telnet Server
	void sendLine(std::string data);
	/// Finish the session
	void closeClient();
	/// Checks the connection timeout
	bool checkTimeout();
	/// Marks timeout to close session
	void markTimeout();

  protected:
	/// Initialise session
	void initialise();
	/// Called every frame/loop by the Terminal Server
	void update();
	/// Statistics variables
	TelnetSessionStats stats;

  private:
	// Returns ip of the peer
	std::string getPeerIP();
	// Write the prompt and any data sat in the input buffer
	void sendPromptAndBuffer();
	// Erase all characters on the current line and move prompt back to beginning of line
	void eraseLine();
	// Echo back message
	void echoBack(char *buffer, u_long length);
	//
	static void stripNVT(std::string &buffer);
	// Remove all escape characters from the line
	static void stripEscapeCharacters(std::string &buffer);
	// Takes backspace commands and removes them and the preceding character from the m_buffer. Handles arrow key
	// actions for history management. Returns true if the input buffer was changed.
	static bool processBackspace(std::string &buffer);
	// Takes tab commands and completes or suggests commands
	bool processTab(std::string &buffer);
	// Add a command into the command history
	void addToHistory(std::string line);
	// Handles arrow key actions for history management. Returns true if the input buffer was changed.
	bool processCommandHistory(std::string &buffer);
	//
	static std::vector<std::string> getCompleteLines(std::string &buffer);

	// Last seen
	std::chrono::system_clock::time_point lastSeenTime;
	// The socket
	Socket m_socket;
	// Parent TelnetServer class
	std::shared_ptr<TelnetServer> m_telnetServer;
	// Buffer of input data (mid line)
	std::string m_buffer;
	// A history of all completed commands
	std::list<std::string> m_history;
	// Iterator to completed commands
	std::list<std::string>::iterator m_historyCursor;

	friend TelnetServer;
};

typedef std::shared_ptr<TelnetSession> SP_TelnetSession;
typedef std::vector<SP_TelnetSession> VEC_SP_TelnetSession;

typedef std::function<void(SP_TelnetSession)> FPTR_ConnectedCallback;
typedef std::function<bool(SP_TelnetSession, std::string)> FPTR_NewLineCallback;
typedef std::function<std::string(SP_TelnetSession, std::string)> FPTR_TabCallback;

class TelnetServer : public std::enable_shared_from_this<TelnetServer>
{
  public:
	/// Constructor for server
	// cppcheck-suppress uninitMemberVar
	TelnetServer() : m_initialised(false), m_promptString(""){};

	/**
	 * @brief Initializes a new Telnet server
	 * @param[in] listenPort Port to listen
	 * @param[in] promptString Prompt string for connected users
	 * @param[in] reg Prometheus registry for stats
	 * @return true If initialized
	 * @return false otherwise
	 */
	bool initialise(u_long listenPort, std::string promptString = "",
					std::shared_ptr<prometheus::Registry> reg = nullptr);

	/// Process new connections and messages
	void update();

	/// Closes the Telnet Server
	void shutdown();

	void connectedCallback(FPTR_ConnectedCallback f) { m_connectedCallback = f; }
	FPTR_ConnectedCallback connectedCallback() const { return m_connectedCallback; }

	void newLineCallback(FPTR_NewLineCallback f) { m_newlineCallback = f; }
	FPTR_NewLineCallback newLineCallBack() const { return m_newlineCallback; }

	void tabCallback(FPTR_TabCallback f) { m_tabCallback = f; }
	FPTR_TabCallback tabCallback() const { return m_tabCallback; }

	VEC_SP_TelnetSession sessions() const { return m_sessions; }

	bool interactivePrompt() const { return m_promptString.length() > 0; }
	void promptString(const std::string &prompt) { m_promptString = prompt; }
	std::string promptString() const { return m_promptString; }

  protected:
	// Called after the telnet session is initialised. function(SP_TelnetSession) {}
	FPTR_ConnectedCallback m_connectedCallback;
	// Called after every new line (from CR or LF) function(SP_TelnetSession, std::string) {}
	FPTR_NewLineCallback m_newlineCallback;
	// Called after TAB detected. function(SP_TelnetSession, std::string, PredictSignalType) {}
	FPTR_TabCallback m_tabCallback;

  private:
	bool acceptConnection();

	u_long m_listenPort;
	Socket m_listenSocket;
	VEC_SP_TelnetSession m_sessions;
	bool m_initialised;
	// A string that denotes the current prompt
	std::string m_promptString;

	// Statistics
	std::unique_ptr<TelnetStats> stats;
};

/**
 * @brief Telnet session connection start callback
 * @param[in] session Handle to session
 */
void TelnetConnectedCallback(SP_TelnetSession session);

/**
 * @brief Telnet session message received callback
 * @param[in] session Handle to session
 * @param[in] line Received message
 */
bool TelnetMessageCallback(SP_TelnetSession session, std::string line);

/**
 * @brief Telnet session TAB received callback
 * @param[in] session Handle to session
 * @param[in] line Received message
 * @return std::string Command to complete
 */
std::string TelnetTabCallback(SP_TelnetSession session, std::string line);
