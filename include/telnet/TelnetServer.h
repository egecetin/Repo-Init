#pragma once

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <vector>

class TelnetServer;
class TelnetSession;

const std::string ANSI_FG_BLACK("\x1b[30m");
const std::string ANSI_FG_RED("\x1b[31m");
const std::string ANSI_FG_GREEN("\x1b[32m");
const std::string ANSI_FG_YELLOW("\x1b[33m");
const std::string ANSI_FG_BLUE("\x1b[34m");
const std::string ANSI_FG_MAGENTA("\x1b[35m");
const std::string ANSI_FG_CYAN("\x1b[36m");
const std::string ANSI_FG_WHITE("\x1b[37m");
const std::string ANSI_FG_DEFAULT("\x1b[39m");

const std::string ANSI_BG_BLACK("\x1b[40m");
const std::string ANSI_BG_RED("\x1b[41m");
const std::string ANSI_BG_GREEN("\x1b[42m");
const std::string ANSI_BG_YELLOW("\x1b[43m");
const std::string ANSI_BG_BLUE("\x1b[44m");
const std::string ANSI_BG_MAGENTA("\x1b[45m");
const std::string ANSI_BG_CYAN("\x1b[46m");
const std::string ANSI_BG_WHITE("\x1b[47m");
const std::string ANSI_BG_DEFAULT("\x1b[49m");

const std::string ANSI_BOLD_ON("\x1b[1m");
const std::string ANSI_BOLD_OFF("\x1b[22m");

const std::string ANSI_ITALICS_ON("\x1b[3m");
const std::string ANSI_ITALCIS_OFF("\x1b[23m");

const std::string ANSI_UNDERLINE_ON("\x1b[4m");
const std::string ANSI_UNDERLINE_OFF("\x1b[24m");

const std::string ANSI_INVERSE_ON("\x1b[7m");
const std::string ANSI_INVERSE_OFF("\x1b[27m");

const std::string ANSI_STRIKETHROUGH_ON("\x1b[9m");
const std::string ANSI_STRIKETHROUGH_OFF("\x1b[29m");

const std::string ANSI_ERASE_LINE("\x1b[2K");
const std::string ANSI_ERASE_SCREEN("\x1b[2J");

const std::string ANSI_ARROW_UP("\x1b\x5b\x41");
const std::string ANSI_ARROW_DOWN("\x1b\x5b\x42");
const std::string ANSI_ARROW_RIGHT("\x1b\x5b\x43");
const std::string ANSI_ARROW_LEFT("\x1b\x5b\x44");

const std::string TELNET_ERASE_LINE("\xff\xf8");

typedef int Socket;

/**
 * @brief Session class for manage connections
 * 
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

	/// For internal unit tests
	static int UNIT_TEST();

  protected:
	//
	void initialise();
	// Called every frame/loop by the Terminal Server
	void update();

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
	// Add a command into the command history
	void addToHistory(std::string line);
	// Handles arrow key actions for history management. Returns true if the input buffer was changed.
	bool processCommandHistory(std::string &buffer);
	//
	static std::vector<std::string> getCompleteLines(std::string &buffer);

	// Last seen
	time_t lastSeenTime;
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
typedef std::function<void(SP_TelnetSession, std::string)> FPTR_NewLineCallback;

class TelnetServer : public std::enable_shared_from_this<TelnetServer>
{
  public:
	/// Constructor for server
	TelnetServer() : m_initialised(false), m_promptString(""){};

	/**
	 * @brief Initializes a new Telnet server
	 *
	 * @param[in] listenPort Port to listen
	 * @param[in] promptString Prompt string for connected users
	 * @return true If initialized
	 * @return false otherwise
	 */
	bool initialise(u_long listenPort, std::string promptString = "");

	/// Process new connections and messages
	void update();

	/// Closes the Telnet Server
	void shutdown();

	void connectedCallback(FPTR_ConnectedCallback f)
	{
		m_connectedCallback = f;
	}
	FPTR_ConnectedCallback connectedCallback() const
	{
		return m_connectedCallback;
	}

	void newLineCallback(FPTR_NewLineCallback f)
	{
		m_newlineCallback = f;
	}
	FPTR_NewLineCallback newLineCallBack() const
	{
		return m_newlineCallback;
	}

	VEC_SP_TelnetSession sessions() const
	{
		return m_sessions;
	}

	bool interactivePrompt() const
	{
		return m_promptString.length() > 0;
	}
	void promptString(std::string prompt)
	{
		m_promptString = prompt;
	}
	std::string promptString() const
	{
		return m_promptString;
	}

  private:
	void acceptConnection();

  private:
	u_long m_listenPort;
	Socket m_listenSocket;
	VEC_SP_TelnetSession m_sessions;
	bool m_initialised;
	// A string that denotes the current prompt
	std::string m_promptString;

  protected:
	// Called after the telnet session is initialised. function(SP_TelnetSession) {}
	FPTR_ConnectedCallback m_connectedCallback;
	// Called after every new line (from CR or LF) function(SP_TelnetSession, std::string) {}
	FPTR_NewLineCallback m_newlineCallback;
};
