#include "Tracer.hpp"

#include "client/crash_report_database.h"
#include "client/settings.h"

#include <algorithm>

std::string Tracer::getSelfExecutableDir()
{
	std::array<char, FILENAME_MAX> pathBuffer;
	auto bytes = std::min(readlink("/proc/self/exe", pathBuffer.data(), sizeof(pathBuffer)),
						 static_cast<ssize_t>(sizeof(pathBuffer) - 1));

    auto path = std::string(pathBuffer.data(), bytes);
    auto lastDelimPos = path.find_last_of('/');
    return (lastDelimPos == std::string::npos) ? "" : path.substr(0, lastDelimPos);
}

Tracer::Tracer(const std::string &serverPath, const std::string &serverProxy, const std::string &crashpadHandlerPath,
				   const std::map<std::string, std::string> &annotations, const std::vector<base::FilePath> &attachments)
	: _annotations(annotations)
{
	auto exeDir = getSelfExecutableDir();

	// Path to crashpad executable
	base::FilePath handler(crashpadHandlerPath.empty() ? exeDir + "/crashpad_handler" : crashpadHandlerPath);

	// Must be writable or crashpad_handler will crash.
	base::FilePath reportsDir(exeDir);
	base::FilePath metricsDir(exeDir);

	// Initialize Crashpad database
	auto database = crashpad::CrashReportDatabase::Initialize(reportsDir);
	if (database == nullptr)
	{
		throw std::runtime_error("Can't initialize crash report database");
	}

	// Enable automated crash uploads
	auto settings = database->GetSettings();
	if (settings == nullptr)
	{
		throw std::runtime_error("Can't get crash report database settings");
	}
	settings->SetUploadsEnabled(true);

	// Start crash handler
	clientHandler = std::make_unique<crashpad::CrashpadClient>();
	if (!clientHandler->StartHandler(handler, reportsDir, metricsDir, serverPath, serverProxy, annotations,
									 {"--no-rate-limit"}, true, false, attachments))
		throw std::runtime_error("Can't start crash handler");
}
