#pragma once

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <unistd.h>

/// Interval of SIGALRM in seconds
constexpr uintmax_t alarmInterval = 1;

/// Main flag to control loops. Can be modified by SIGINT
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
extern volatile bool loopFlag;

/// Flags and definitions for runtime checks
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
extern std::vector<std::pair<std::string, std::shared_ptr<std::atomic_flag>>> vCheckFlag;
/* ################################################################################### */
/* ############################# MAKE MODIFICATIONS HERE ############################# */
/* ################################################################################### */

/* ################################################################################### */
/* ################################ END MODIFICATIONS ################################ */
/* ################################################################################### */

/**
 * @class InputParser
 * @brief Parses command line inputs
 */
class InputParser {
  private:
	std::vector<std::string> tokens;

  public:
	/**
	 * @brief Constructs a new InputParser object
	 * @param[in] argc Number of input arguments
	 * @param[in] argv Input arguments
	 */
	InputParser(const int &argc, char **argv)
	{
		for (int i = 1; i < argc; ++i)
		{
			this->tokens.emplace_back(argv[i]);
		}
		this->tokens.emplace_back("");
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
		static const std::string empty_string;
		return empty_string;
	}

	/**
	 * @brief Checks whether provided command line option exists.
	 * @param[in] option Option to check
	 * @return true If the provided option is found
	 * @return false If the provided option is not found
	 */
	bool cmdOptionExists(const std::string &option) const
	{
		return std::find(this->tokens.begin(), this->tokens.end(), option) != this->tokens.end();
	}
};

/**
 * @struct spinlock
 * @brief Spinlock implementation from https://rigtorp.se/spinlock/
 */
struct spinlock {
  private:
	std::atomic<bool> lock_ = {false};

  public:
	/**
	 * @brief Locks the spinlock
	 */
	void lock() noexcept
	{
		for (;;)
		{
			// Optimistically assume the lock is free on the first try
			if (!lock_.exchange(true, std::memory_order_acquire))
			{
				return;
			}
			// Wait for lock to be released without generating cache misses
			while (lock_.load(std::memory_order_relaxed))
			{
				// Issue X86 PAUSE or ARM YIELD instruction to reduce contention between
				// hyper-threads
				__builtin_ia32_pause();
			}
		}
	}

	/**
	 * @brief Tries to lock the spinlock
	 * @return true If the lock is acquired successfully
	 * @return false If the lock is already acquired by another thread
	 */
	bool try_lock() noexcept
	{
		// First do a relaxed load to check if lock is free in order to prevent
		// unnecessary cache misses if someone does while(!try_lock())
		return !lock_.load(std::memory_order_relaxed) && !lock_.exchange(true, std::memory_order_acquire);
	}

	/**
	 * @brief Unlocks the spinlock
	 */
	void unlock() noexcept { lock_.store(false, std::memory_order_release); }
};

/**
 * @brief Read initial config from JSON and verify all required entries exist
 * @param[in] dir Path to JSON
 * @return true If all variables are read successfully
 * @return false If some variables cannot be found
 */
bool readAndVerifyConfig(const std::string &dir);

/**
 * @brief Reads a single entry from the config
 * @param[in] dir Path to JSON
 * @param[in] value Value to read
 * @return std::string Read value
 */
std::string readSingleConfig(const std::string &dir, const std::string &value);

/**
 * @brief Converts errno to a readable string
 * @param[in] errVal errno value
 * @return std::string Error message
 */
std::string getErrnoString(int errVal);

/**
 * @brief Gets the value of an environment variable
 * @param[in] key Requested variable name
 * @return std::string Value of the variable
 */
std::string getEnvVar(const std::string &key);

/**
 * @brief Searches line patterns from a file
 * @param[in] filePath Path to the file
 * @param[in] pattern Regex search pattern
 * @return std::vector<std::string> Matched lines
 */
std::vector<std::string> findFromFile(const std::string &filePath, const std::string &pattern);

/**
 * @brief Searches line patterns from a file
 * @param[in] filePath Path to the file
 * @param[in] pattern Regex search pattern
 * @param[out] lastWord Last word (space delimiter) of the first found line
 * @return std::vector<std::string> Matched lines
 */
std::vector<std::string> findFromFile(const std::string &filePath, const std::string &pattern, std::string &lastWord);

/**
 * @brief Function invoked by SIGALRM signal
 * @param[in] signum Signal indicator
 */
void alarmFunc(int signum);

/**
 * @brief Function invoked by user interruption signals
 * @param[in] signum Signal indicator
 */
void interruptFunc(int signum);
