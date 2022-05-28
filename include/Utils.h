#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include <unistd.h>

#define CONFIG_FILE_PATH "config.json"

// Config variables
extern uintmax_t ALARM_INTERVAL;

extern int ZMQ_RECV_TIMEOUT;
extern int ZMQ_SEND_TIMEOUT;
extern std::string CONTROL_IPC_PATH;

extern volatile time_t currentTime;
extern volatile uintmax_t alarmCtr;
extern volatile bool loopFlag;
extern volatile bool sigReadyFlag;

class InputParser
{
  public:
	InputParser(int &argc, char **argv)
	{
		for (int i = 1; i < argc; ++i)
			this->tokens.push_back(std::string(argv[i]));
	}

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

	bool cmdOptionExists(const std::string &option) const
	{
		return std::find(this->tokens.begin(), this->tokens.end(), option) != this->tokens.end();
	}

  private:
	std::vector<std::string> tokens;
};

/**
 * @brief Prints the version
 *
 */
void print_version(void);

/**
 * @brief Read initial config from JSON
 *
 * @param dir       Path to JSON
 * @return true     Read all variables
 * @return false    Can't find some variables
 */
bool readConfig(const char *dir);

/**
 * @brief Reads a single entry from config
 *
 * @param dir 			Path to JSON
 * @param value 		Value to read
 * @return std::string 	Read value
 */
std::string readSingleConfig(const char *dir, std::string value);

/**
 * @brief SIGALRM function
 *
 * @param signum
 */
void alarmFunc(int signum);

/**
 * @brief Abnormal signal function
 *
 * @param signum
 */
void backtracer(int signum);

/**
 * @brief User interrupt function
 *
 * @param signum
 */
void interruptFunc(int signum);
