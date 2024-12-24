#pragma once

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
	std::string _filePath;

  public:
	/**
	 * Constructor
	 * @param[in] path Path to the file
	 */
	explicit FileLocker(const std::string &path) : _filePath(path)
	{
		// Open file to get descriptor
		_fd = open(path.c_str(), O_RDWR, 0666);
		if (_fd < 0)
		{
			throw std::runtime_error("Can't open file " + _filePath + ": " + getErrnoString(errno));
		}

		// Acquire lock
		if (flock(_fd, LOCK_SH | LOCK_NB) != 0)
		{
			if (close(_fd) != 0)
			{
				throw std::runtime_error("Can't get lock and also can't close file " + _filePath + ": " +
										 getErrnoString(errno));
			}
			throw std::runtime_error("Can't get lock for file " + _filePath + ": " + getErrnoString(errno));
		}
	}

	/**
	 * Destructor
	 */
	~FileLocker()
	{
		// Unlock and close
		if (_fd >= 0)
		{
			flock(_fd, LOCK_UN);
			close(_fd);
		}
	}
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
	int _wDecsriptor;
	/// File path
	std::filesystem::path _filePath;
	/// Callback function
	FNotifyCallback _notifyCallback;

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
	FileMonitor(const std::filesystem::path &filePath, const int notifyEvents = IN_MODIFY);

	FNotifyCallback notifyCallback() const { return _notifyCallback; }
	void notifyCallback(FNotifyCallback func) { _notifyCallback = std::move(func); }

	/**
	 * Destructor
	 */
	~FileMonitor();
};
