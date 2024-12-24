#include "utils/FileHelpers.hpp"

#include <unistd.h>

void FileMonitor::threadFunction()
{
}

FileMonitor::FileMonitor(const std::filesystem::path &filePath, const int notifyEvents) : _filePath(filePath)
{
	_fDescriptor = inotify_init();
	if (_fDescriptor < 0)
	{
		throw std::runtime_error("Failed to initialize inotify");
	}

    _wDecsriptor = inotify_add_watch(_fDescriptor, _filePath.c_str(), notifyEvents);
    if (_wDecsriptor < 0)
    {
        close(_fDescriptor);
        throw std::runtime_error("Failed to add watch descriptor");
    }

    _thread = std::make_unique<std::thread>(&FileMonitor::threadFunction, this);
}