#include "Utils.hpp"
#include "Version.h"

#include <csignal>
#include <execinfo.h>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/writer.h>
#include <spdlog/spdlog.h>

volatile uintmax_t alarmCtr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
volatile bool loopFlag;		 // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
/* ################################################################################### */
/* ############################# MAKE MODIFICATIONS HERE ############################# */
/* ################################################################################### */

/* ################################################################################### */
/* ################################ END MODIFICATIONS ################################ */
/* ################################################################################### */

// GCOVR_EXCL_START
void print_version()
{
	spdlog::info("{:<15}: v{} {} {} {}", PROJECT_NAME, PROJECT_FULL_REVISION, BUILD_TYPE, PROJECT_BUILD_DATE,
				 PROJECT_BUILD_TIME);
	/* ################################################################################### */
	/* ############################# MAKE MODIFICATIONS HERE ############################# */
	/* ################################################################################### */

	/* ################################################################################### */
	/* ################################ END MODIFICATIONS ################################ */
	/* ################################################################################### */
}
// GCOVR_EXCL_STOP

// GCOVR_EXCL_START
std::string get_version()
{
	return std::string("v") + PROJECT_FULL_REVISION + " " + BUILD_TYPE + " " + PROJECT_BUILD_DATE + " " +
		   PROJECT_BUILD_TIME;
}
// GCOVR_EXCL_STOP

template <typename T> std::string stringify(const T &o)
{
	rapidjson::StringBuffer sb;
	rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
	o.Accept(writer);
	return sb.GetString();
}

bool readConfig(const std::string &dir)
{
	// NOLINTBEGIN
	FILE *fptr = fopen(dir.c_str(), "r");
	if (fptr == nullptr)
	{
		spdlog::critical("Can't open config file");
		return false;
	}

	char buffer[65536];
	rapidjson::FileReadStream iFile(fptr, buffer, sizeof(buffer));

	rapidjson::Document doc;
	doc.ParseStream(iFile);
	fclose(fptr);

	// Check is there any data
	if (doc.IsNull())
	{
		spdlog::critical("Read config is empty");
		return false;
	}

	// Check variables exist
	/* ################################################################################### */
	/* ############################# MAKE MODIFICATIONS HERE ############################# */
	/* ################################################################################### */
	std::vector<std::string> list = {};
	/* ################################################################################### */
	/* ################################ END MODIFICATIONS ################################ */
	/* ################################################################################### */

	spdlog::debug("Reading variables from config ...");
	// cppcheck-suppress knownEmptyContainer
	for (const auto &entry : list)
	{
		if (!doc.HasMember(entry.c_str()))
		{
			spdlog::critical("Can't read {}", entry);
			return false;
		}
		spdlog::debug("{}: {}", entry, stringify(doc[entry.c_str()]));
	}
	spdlog::debug("All variables read");

	// Set variables
	/* ################################################################################### */
	/* ############################# MAKE MODIFICATIONS HERE ############################# */
	/* ################################################################################### */

	/* ################################################################################### */
	/* ################################ END MODIFICATIONS ################################ */
	/* ################################################################################### */

	// Cleanup
	doc.GetAllocator().Clear();
	doc.RemoveAllMembers();

	return true;
	// NOLINTEND
}

std::string readSingleConfig(const std::string &dir, std::string value)
{
	// NOLINTBEGIN
	FILE *fptr = fopen(dir.c_str(), "r");
	if (fptr == nullptr)
	{
		spdlog::critical("Can't open config file");
		return "";
	}

	char buffer[65536];
	rapidjson::FileReadStream iFile(fptr, buffer, sizeof(buffer));

	rapidjson::Document doc;
	doc.ParseStream(iFile);
	fclose(fptr);

	// Check is there any data
	if (doc.IsNull())
	{
		spdlog::critical("Read config is empty");
		return "";
	}

	if (doc.HasMember(value.c_str()))
	{
		spdlog::debug("Read config {}: {}", value, stringify(doc[value.c_str()]));
		if (doc[value.c_str()].IsString())
		{
			return doc[value.c_str()].GetString();
		}
		return stringify(doc[value.c_str()]);
	}
	spdlog::error("Can't find requested field at config {}", value);
	return "";
	// NOLINTEND
}

// GCOVR_EXCL_START
void alarmFunc(int /*unused*/)
{
	++alarmCtr;
	if (loopFlag)
	{
		alarm(alarmInterval);
	}
}
// GCOVR_EXCL_STOP

// GCOVR_EXCL_START
void interruptFunc(int /*unused*/)
{
	if (loopFlag)
	{
		loopFlag = false;
	}
	else
	{
		(void)!write(STDERR_FILENO, "Interrupt in progress...\n", 26); // NOLINT(readability-implicit-bool-conversion)
	}
}
// GCOVR_EXCL_STOP
