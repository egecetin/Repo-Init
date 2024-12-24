#pragma once

#include <atomic>
#include <filesystem>
#include <fstream>
#include <regex>
#include <string>
#include <sys/inotify.h>
#include <thread>
#include <vector>

/**
 * Searches line patterns from a file
 * @param[in] filePath Path to the file
 * @param[in] pattern Regex search pattern
 * @param[out] lastWord Last word (space delimiter) of the first found line
 * @return std::vector<std::string> Matched lines
 */
inline std::vector<std::string> findFromFile(const std::string &filePath, const std::string &pattern,
											 std::string &lastWord)
{
	const std::regex regExp(pattern);
	std::ifstream inFile(filePath);
	std::vector<std::string> matchedLines;

	std::string readLine;
	while (getline(inFile, readLine))
	{
		if (std::regex_search(readLine, regExp))
		{
			matchedLines.push_back(readLine);
		}
	}

	if (!matchedLines.empty())
	{
		auto pos = matchedLines.front().find_last_of(' ');
		if (pos != std::string::npos && pos != matchedLines.front().size())
		{
			lastWord = matchedLines.front().substr(pos + 1);
		}
	}

	return matchedLines;
}

/**
 * Searches line patterns from a file
 * @param[in] filePath Path to the file
 * @param[in] pattern Regex search pattern
 * @return std::vector<std::string> Matched lines
 */
inline std::vector<std::string> findFromFile(const std::string &filePath, const std::string &pattern)
{
	std::string lastWord;
	return findFromFile(filePath, pattern, lastWord);
}

/**
 * Locks a file in constructor and unlocks in destructor
 */
class FileLocker {
  private:
	/// File descriptor
	int _fd;
	/// Path to file
	std::filesystem::path _filePath;

  public:
	/**
	 * Constructor
	 * @param[in] path Path to the file
	 */
	explicit FileLocker(const std::filesystem::path &path);

	/**
	 * Destructor
	 */
	~FileLocker();
};

/// Callback function for file notifications
using FNotifyCallback = std::function<void(const void *)>;

/**
 * Invokes functions for a file for given notify events
 */
class FileMonitor {
  private:
	/// File descriptor
	int _fDescriptor;
	/// Watch descriptor
	int _wDescriptor;
	/// File path
	std::filesystem::path _filePath;
	/// Callback function
	FNotifyCallback _notifyCallback;
	/// Notify types
	int _notifyEvents;
	/// User pointer
	const void *_userPtr = nullptr;

	/// Thread
	std::unique_ptr<std::thread> _thread;
	/// Flag to stop monitoring
	std::atomic_flag _shouldStop{false};

	void threadFunction() noexcept;

  public:
	/**
	 * Constructor
	 * @param[in] filePath Path to the file
	 * @param[in] notifyEvents Events to notify
	 */
	FileMonitor(const std::filesystem::path &filePath, int notifyEvents = IN_MODIFY);

	FNotifyCallback notifyCallback() const { return _notifyCallback; }
	void notifyCallback(FNotifyCallback func) { _notifyCallback = std::move(func); }

	/**
	 * Sets user pointer
	 * @param[in] ptr User pointer
	 */
	void userPtr(const void *ptr) { _userPtr = ptr; }

	/**
	 * Destructor
	 */
	~FileMonitor();
};
