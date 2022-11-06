#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include <unistd.h>

/// Variable to define path to config file
#define CONFIG_FILE_PATH "config.json"

// Config variables

/// Interval of SIGALRM in seconds
extern uintmax_t ALARM_INTERVAL;
/// Interval of Heartbeat in seconds
extern uintmax_t HEARTBEAT_INTERVAL;

/// Receive timeout of ZMQ sockets. Read from config at startup
extern int ZMQ_RECV_TIMEOUT;
/// Send timeout of ZMQ sockets. Read from config at startup
extern int ZMQ_SEND_TIMEOUT;
/// Interprocess path of controller thread
extern std::string CONTROL_IPC_PATH;

/// Port number to Telnet server
extern uint16_t TELNET_PORT;
/// Bind address of Prometheus service
extern std::string PROMETHEUS_ADDR;
/* ############################# MAKE MODIFICATIONS HERE ############################# */

/* ################################ END MODIFICATIONS ################################ */

/// Current time value set by SIGALRM
extern volatile time_t currentTime;
/// Alarm counter to track. Incremented by SIGALRM
extern volatile uintmax_t alarmCtr;
/// Main flag to control loops. Can be modified by SIGINT
extern volatile bool loopFlag;
/* ############################# MAKE MODIFICATIONS HERE ############################# */

/* ################################ END MODIFICATIONS ################################ */

/**
 * @brief Parses command line inputs
 */
class InputParser
{
  public:
	/**
	 * @brief Constructs a new InputParser object
	 * @param[in] argc Number of input arguments
	 * @param[in] argv Input arguments
	 */
	InputParser(const int &argc, char **argv)
	{
		for (int i = 1; i < argc; ++i)
			this->tokens.push_back(std::string(argv[i]));
		this->tokens.push_back(std::string(""));
	}

	/**
	 * @brief Gets single command line input
	 * @param[in] option Option to check
	 * @return const std::string& Found command line input. Empty string if not found
	 */
	const std::string &getCmdOption(const std::string &option) const
	{
		std::vector<std::string>::const_iterator itr;
		itr = std::find(this->tokens.begin(), this->tokens.end(), option);
		if (itr != this->tokens.end() && ++itr != this->tokens.end())
		{
			return *itr;
		}
		static const std::string empty_string("");
		return empty_string;
	}

	/**
	 * @brief Checks whether provided command line option is exists.
	 * @param[in] option Option to check
	 * @return true If the provided option is found
	 * @return false If the provided option is not found
	 */
	bool cmdOptionExists(const std::string &option) const
	{
		return std::find(this->tokens.begin(), this->tokens.end(), option) != this->tokens.end();
	}

  private:
	std::vector<std::string> tokens;
};

/**
 * @brief Prints the version
 */
void print_version(void);

/**
 * @brief Initializes the logger
 * @param argc
 * @param argv
 * @return true On success
 * @return false otherwise
 */
bool init_logger(int argc, char **argv);

/**
 * @brief Closes and flushes the logger
 */
void close_logger(void);

/**
 * @brief Read initial config from JSON
 * @param[in] dir	Path to JSON
 * @return true    	Read all variables
 * @return false    Can't find some variables
 */
bool readConfig(const char *dir);

/**
 * @brief Reads a single entry from config
 * @param[in] dir 		Path to JSON
 * @param[in] value 	Value to read
 * @return std::string 	Read value
 */
std::string readSingleConfig(const char *dir, std::string value);

/**
 * @brief Function invoked by SIGALRM signal
 * @param[in] signum Signal indicator
 */
void alarmFunc(int signum);

/**
 * @brief Function invoked by abnormal (e.g. SIGSEGV) signals
 * @param[in] signum Signal indicator
 */
void backtracer(int signum);

/**
 * @brief Function invoked by user interruption signals
 * @param[in] signum Signal indicator
 */
void interruptFunc(int signum);
