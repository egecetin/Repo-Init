#include "trace/Crashpad.hpp"

#include "client/crash_report_database.h"
#include "client/settings.h"

#include <algorithm>
#include <cstring>

std::string Crashpad::getExecutableDir()
{
	char pathBuffer[FILENAME_MAX];
	int bytes = std::min(readlink("/proc/self/exe", pathBuffer, sizeof(pathBuffer)),
						 static_cast<ssize_t>(sizeof(pathBuffer) - 1));
	if (bytes >= 0)
	{
		pathBuffer[bytes] = '\0';
	}

	char *lastForwardSlash = strrchr(&pathBuffer[0], '/');
	if (lastForwardSlash == NULL)
		return NULL;
	*lastForwardSlash = '\0';

	return pathBuffer;
}

Crashpad::Crashpad(const std::string &serverPath, const std::string &serverProxy, const std::string &crashpadHandlerPath,
				   const std::map<std::string, std::string> &annotations, const std::vector<base::FilePath> &attachments)
	: _annotations(annotations)
{
	std::string exeDir = getExecutableDir();

	// Path to crashpad executable
	base::FilePath handler(crashpadHandlerPath.empty() ? exeDir + "/crashpad_handler" : crashpadHandlerPath);

	// Must be writable or crashpad_handler will crash.
	base::FilePath reportsDir(exeDir);
	base::FilePath metricsDir(exeDir);

	// Initialize Crashpad database
	std::unique_ptr<crashpad::CrashReportDatabase> database = crashpad::CrashReportDatabase::Initialize(reportsDir);
	if (database == nullptr)
		throw std::runtime_error("Can't initialize crash report database");

	// Enable automated crash uploads
	crashpad::Settings *settings = database->GetSettings();
	if (settings == nullptr)
		throw std::runtime_error("Can't get crash report database settings");
	settings->SetUploadsEnabled(true);

	// Start crash handler
	clientHandler = std::make_unique<crashpad::CrashpadClient>();
	if (!clientHandler->StartHandler(handler, reportsDir, metricsDir, serverPath, serverProxy, annotations,
									 {"--no-rate-limit"}, true, true, attachments))
		throw std::runtime_error("Can't start crash handler");
}