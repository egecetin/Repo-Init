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

/* ################################################################################### */
/* ############################# MAKE MODIFICATIONS HERE ############################# */
/* ################################################################################### */

/* ################################################################################### */
/* ################################ END MODIFICATIONS ################################ */
/* ################################################################################### */

/// Main flag to control loops. Can be modified by SIGINT
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
extern volatile bool loopFlag;
/// Flags and definitions for runtime checks
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
extern std::vector<std::pair<std::string, std::unique_ptr<std::atomic_flag>>> vCheckFlag;
/* ################################################################################### */
/* ############################# MAKE MODIFICATIONS HERE ############################# */
/* ################################################################################### */

/* ################################################################################### */
/* ################################ END MODIFICATIONS ################################ */
/* ################################################################################### */

/**
 * @brief Parses command line inputs
 */
class InputParser {
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
			this->tokens.emplace_back(argv[i]); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
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
 * @brief Spinlock implementation from https://rigtorp.se/spinlock/
 */
struct spinlock {
  private:
	std::atomic<bool> lock_ = {false};

  public:
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

	bool try_lock() noexcept
	{
		// First do a relaxed load to check if lock is free in order to prevent
		// unnecessary cache misses if someone does while(!try_lock())
		return !lock_.load(std::memory_order_relaxed) && !lock_.exchange(true, std::memory_order_acquire);
	}

	void unlock() noexcept { lock_.store(false, std::memory_order_release); }
};

/**
 * @brief Prints the version
 */
void print_version();

/**
 * @brief Returns the version
 * @return std::string Version string
 */
std::string get_version();

/**
 * @brief Read initial config from JSON
 * @param[in] dir	Path to JSON
 * @return true    	Read all variables
 * @return false    Can't find some variables
 */
bool readConfig(const std::string &dir);

/**
 * @brief Reads a single entry from config
 * @param[in] dir 		Path to JSON
 * @param[in] value 	Value to read
 * @return std::string 	Read value
 */
std::string readSingleConfig(const std::string &dir, const std::string &value);

/**
 * @brief Converts errno to readable string
 * @param[in] errVal errno value
 * @return std::string Error message
 */
std::string getErrnoString(int errVal);

/**
 * @brief Get the environment variable
 * @param[in] key Requested variable name
 * @return std::string Value of the variable
 */
std::string getEnvVar(const std::string &key);

/**
 * @brief Searches line patterns from a file
 * @param[in] filePath Path to file
 * @param[in] pattern Regex search pattern
 * @return std::vector<std::string> Matched lines
 */
std::vector<std::string> findFromFile(const std::string &filePath, const std::string &pattern);

/**
 * @brief Searches line patterns from a file
 * @param[in] filePath Path to file
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
