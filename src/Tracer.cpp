#include "Tracer.hpp"

#include "client/crash_report_database.h"
#include "client/crashpad_client.h"
#include "client/settings.h"

#include <algorithm>
#include <unistd.h>
#include <sys/socket.h>

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

	int result = getsockopt(sockId, SOL_SOCKET, SO_ERROR, &error, &len);
	if (result == 0 && error == 0 && recv(sockId, NULL, 1, MSG_PEEK | MSG_DONTWAIT) != 0)
	{
		return true;
	}
	return false;
}

std::string Tracer::getSelfExecutableDir()
{
	std::array<char, FILENAME_MAX> pathBuffer{'\0'};
	auto bytes = readlink("/proc/self/exe", pathBuffer.data(), sizeof(pathBuffer));

	auto path = std::string(pathBuffer.data(), bytes == -1 ? 0 : static_cast<size_t>(bytes));
	auto lastDelimPos = path.find_last_of('/');
	return (lastDelimPos == std::string::npos) ? "" : path.substr(0, lastDelimPos);
}

Tracer::Tracer(const std::string &serverPath, const std::string &serverProxy, const std::string &crashpadHandlerPath,
			   const std::map<std::string, std::string> &annotations, const std::vector<base::FilePath> &attachments,
			   const std::string &reportPath)
	: _serverPath(serverPath), _serverProxy(serverProxy), _annotations(annotations), _attachments(attachments)
{
	auto selfDir = getSelfExecutableDir();

	_handlerPath = crashpadHandlerPath.empty() ? selfDir + "/crashpad_handler" : crashpadHandlerPath;
	_reportPath = reportPath.empty() ? selfDir : reportPath;
	_clientHandler = std::make_unique<crashpad::CrashpadClient>();

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

	if(sockId > 0 && !checkSocketIsRunning(sockId))
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
		startHandler();
}