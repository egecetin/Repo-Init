#include "Tracer.hpp"
#include "Utils.hpp"

#include "client/crash_report_database.h"
#include "client/crashpad_client.h"
#include "client/settings.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void Tracer::startHandler()
{
	// Path to crashpad executable
	base::FilePath handler(_handlerPath);

	// Must be writable or crashpad_handler will crash
	base::FilePath reportsDir(_reportPath);
	base::FilePath metricsDir(_reportPath);

	// Initialize Crashpad database
	auto database = crashpad::CrashReportDatabase::Initialize(reportsDir);
	if (database == nullptr)
	{
		throw std::runtime_error("Can't initialize crash report database");
	}

	// Enable automated crash uploads
	auto *settings = database->GetSettings();
	if (settings == nullptr)
	{
		throw std::runtime_error("Can't get crash report database settings");
	}
	settings->SetUploadsEnabled(true);

	// Start crash handler
	if (!_clientHandler->StartHandler(handler, reportsDir, metricsDir, _serverPath, _serverProxy, _annotations,
									  {"--no-rate-limit"}, true, false, _attachments))
	{
		throw std::runtime_error("Can't start crash handler");
	}
}

bool Tracer::checkPidIsRunning(pid_t processId) { return kill(processId, 0) == 0; }

bool Tracer::checkSocketIsRunning(int sockId)
{
	int error = 0;
	socklen_t len = sizeof(error);

	char buff = 0;
	int result = getsockopt(sockId, SOL_SOCKET, SO_ERROR, &error, &len);
	return result == 0 && error == 0 && recv(sockId, &buff, 1, MSG_PEEK | MSG_DONTWAIT) != 0;
}

std::string Tracer::getSelfExecutableDir()
{
	std::array<char, FILENAME_MAX> pathBuffer{'\0'};
	auto bytes = readlink("/proc/self/exe", pathBuffer.data(), sizeof(pathBuffer));

	auto path = std::string(pathBuffer.data(), bytes == -1 ? 0 : static_cast<size_t>(bytes));
	auto lastDelimPos = path.find_last_of('/');
	return (lastDelimPos == std::string::npos) ? "" : path.substr(0, lastDelimPos);
}

bool Tracer::createDir(const std::string &path)
{
	struct stat info;

	if (stat(path.c_str(), &info) != 0 && errno == ENOENT)
	{
		return mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
	}
	return S_ISDIR(info.st_mode);
}

Tracer::Tracer(std::string serverPath, std::string serverProxy, const std::string &crashpadHandlerPath,
			   std::map<std::string, std::string> annotations, std::vector<base::FilePath> attachments,
			   const std::string &reportPath)
	: _serverPath(std::move(serverPath)), _serverProxy(std::move(serverProxy)), _annotations(std::move(annotations)),
	  _attachments(std::move(attachments))
{
	auto selfDir = getSelfExecutableDir();

	_handlerPath = crashpadHandlerPath.empty() ? selfDir + "/crashpad_handler" : crashpadHandlerPath;
	_reportPath = reportPath.empty() ? selfDir : reportPath;
	_clientHandler = std::make_unique<crashpad::CrashpadClient>();

	if (!createDir(_reportPath))
	{
		throw std::invalid_argument("Can't create report directory " + _reportPath + ": " + getErrnoString(errno));
	}

	startHandler();
}

bool Tracer::isRunning()
{
	int sockId{-1};
	pid_t processId{-1};

	if (!_clientHandler->GetHandlerSocket(&sockId, &processId))
	{
		return false;
	}

	if (sockId >= 0 && !checkSocketIsRunning(sockId))
	{
		return false;
	}
	if (processId > 0 && !checkPidIsRunning(processId))
	{
		return false;
	}
	return true;
}

void Tracer::restart()
{
	if (!isRunning())
	{
		startHandler();
	}
}

bool Tracer::dumpSharedLibraryInfo(const std::string &filePath)
{
	// Open the output file
	std::ofstream ofile(filePath);
	if (!ofile.is_open())
	{
		return false;
	}

	// Get the shared library information
	std::ifstream maps("/proc/self/maps");

	std::string line;
	while (std::getline(maps, line))
	{
		// The format of each line is: address perms offset dev inode pathname
		// We only care about the address and the pathname, which are the first and last fields
		std::istringstream iss(line);
		std::string address;
		std::string perms;
		std::string offset;
		std::string dev;
		std::string inode;
		std::string pathname;
		iss >> address >> perms >> offset >> dev >> inode >> pathname;

		// We only want the shared libraries, which have the .so extension and the read and execute permissions
		if (pathname.find(".so") != std::string::npos && perms.find("r-x") != std::string::npos)
		{
			// The address field is in the form of start-end, we only need the start address
			std::string start = address.substr(0, address.find('-'));

			// Convert the start address from hexadecimal string to unsigned long
			unsigned long addr = std::stoul(start, nullptr, 16);

			ofile << pathname << " " << addr << std::endl;
		}
	}

	return true;
}
