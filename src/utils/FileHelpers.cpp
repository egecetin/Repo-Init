#include "utils/FileHelpers.hpp"

#include "utils/ErrorHelpers.hpp"

#include <fcntl.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <spdlog/spdlog.h>

/// Sleep interval for the file monitor
constexpr int SLEEP_INTERVAL_MS = 50;

void FileMonitor::threadFunc() noexcept
{
	while (!_shouldStop._M_i)
	{
		// Buffer for reading events
		unsigned int nBytes;
		if (ioctl(_fDescriptor, FIONREAD, &nBytes) < 0)
		{
			spdlog::error("Failed to get available events for file monitoring: {}", getErrnoString(errno));
		}

		auto buffer = std::make_unique<char[]>(nBytes);
		auto nRead = read(_fDescriptor, buffer.get(), nBytes);
		if (nRead < 0)
		{
			spdlog::error("Failed to read events for file monitoring: {}", getErrnoString(errno));
		}

		ssize_t idx = 0;
		while (_notifyCallback && idx < nRead)
		{
			const auto *event = reinterpret_cast<inotify_event *>(&buffer[idx]);

			// Check if file notify type matches
			if (event->mask & _notifyEvents)
			{
				_notifyCallback(_userPtr);
				break;
			}

			idx += sizeof(inotify_event) + event->len;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_INTERVAL_MS));
	}
}

FileMonitor::FileMonitor(const std::filesystem::path &filePath, int notifyEvents)
	: _filePath(filePath), _notifyEvents(notifyEvents)
{
	_fDescriptor = inotify_init();
	if (_fDescriptor < 0)
	{
		throw std::ios_base::failure("Failed to initialize inotify");
	}

	_wDescriptor = inotify_add_watch(_fDescriptor, _filePath.c_str(), notifyEvents);
	if (_wDescriptor < 0)
	{
		close(_fDescriptor);
		throw std::ios_base::failure("Failed to add watch descriptor");
	}

	_thread = std::make_unique<std::thread>(&FileMonitor::threadFunc, this);
}

FileMonitor::~FileMonitor()
{
	_shouldStop.test_and_set();
	if (_thread && _thread->joinable())
	{
		_thread->join();
		_thread.reset();
	}

	// Remove watch descriptor first
    if (_wDescriptor >= 0)
    {
        if (inotify_rm_watch(_fDescriptor, _wDescriptor) < 0) {
            spdlog::error("Failed to remove watch descriptor: {}", getErrnoString(errno));
        }
        _wDescriptor = -1;
    }

    // Then close the file descriptor
    if (_fDescriptor >= 0)
    {
        if (close(_fDescriptor) < 0) {
			spdlog::error("Failed to close file descriptor: {}", getErrnoString(errno));
        }
        _fDescriptor = -1;
    }
}
