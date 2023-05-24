#include "Utils.hpp"
#include "Version.h"

#include <array>
#include <csignal>
#include <execinfo.h>
#include <fstream>
#include <regex>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/writer.h>
#include <spdlog/spdlog.h>

volatile bool loopFlag; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
/* ################################################################################### */
/* ############################# MAKE MODIFICATIONS HERE ############################# */
/* ################################################################################### */
std::string configPath; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
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

bool tryOpenAndParseConfig(const std::string &dir, rapidjson::Document &doc)
{
	std::ifstream inFile(dir);
	rapidjson::IStreamWrapper fStreamWrapper(inFile);

	doc.ParseStream(fStreamWrapper);

	// Check is there any data
	if (doc.IsNull())
	{
		spdlog::critical("Read config is empty");
		return false;
	}

	return true;
}

bool readConfig(const std::string &dir)
{
	rapidjson::Document doc;
	if (!tryOpenAndParseConfig(dir, doc))
	{
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
}

std::string readSingleConfig(const std::string &dir, std::string value)
{
	rapidjson::Document doc;
	if (!tryOpenAndParseConfig(dir, doc))
	{
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
}

std::string getErrnoString(int errVal)
{
	std::array<char, BUFSIZ> buffer{};
	return strerror_r(errVal, buffer.data(), BUFSIZ);
}

std::string getEnvVar(const std::string &key)
{
	const char *val = getenv(key.c_str()); // NOLINT(concurrency-mt-unsafe)
	return val == nullptr ? std::string("") : std::string(val);
}

std::vector<std::string> findFromFile(const std::string &filePath, const std::string &pattern)
{
	std::string lastWord;
	return findFromFile(filePath, pattern, lastWord);
}

std::vector<std::string> findFromFile(const std::string &filePath, const std::string &pattern, std::string &lastWord)
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

// GCOVR_EXCL_START
void alarmFunc(int /*unused*/)
{
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
