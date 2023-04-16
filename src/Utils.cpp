#include "Utils.hpp"
#include "Version.h"

#include <csignal>
#include <execinfo.h>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/writer.h>
#include <spdlog/spdlog.h>

std::string CONFIG_FILE_PATH = "config.json";

uintmax_t ALARM_INTERVAL;
uintmax_t HEARTBEAT_INTERVAL;

int ZMQ_SEND_TIMEOUT;
int ZMQ_RECV_TIMEOUT;
std::string ZEROMQ_SERVER_PATH;

uint16_t TELNET_PORT;
std::string PROMETHEUS_ADDR;
std::string HEARTBEAT_INDEX;
/* ################################################################################### */
/* ############################# MAKE MODIFICATIONS HERE ############################# */
/* ################################################################################### */

/* ################################################################################### */
/* ################################ END MODIFICATIONS ################################ */
/* ################################################################################### */

volatile uintmax_t alarmCtr;
volatile bool loopFlag;
/* ################################################################################### */
/* ############################# MAKE MODIFICATIONS HERE ############################# */
/* ################################################################################### */

/* ################################################################################### */
/* ################################ END MODIFICATIONS ################################ */
/* ################################################################################### */

// GCOVR_EXCL_START
void print_version(void)
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
std::string get_version(void)
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
	FILE *fptr = fopen(dir.c_str(), "r");
	if (!fptr)
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
	std::vector<std::string> list = {"ZMQ_RECV_TIMEOUT", "ZMQ_SEND_TIMEOUT", "ZEROMQ_SERVER_PATH"};
	/* ################################################################################### */
	/* ################################ END MODIFICATIONS ################################ */
	/* ################################################################################### */

	spdlog::debug("Reading variables from config ...");
	for (const auto &entry : list)
	{
		if (!doc.HasMember(entry.c_str()))
		{
			spdlog::critical("Can't read {}", entry);
			return false;
		}
		else
			spdlog::debug("{}: {}", entry, stringify(doc[entry.c_str()]));
	}
	spdlog::debug("All variables read");

	// Set variables
	ZMQ_RECV_TIMEOUT = doc["ZMQ_RECV_TIMEOUT"].GetUint64();
	ZMQ_SEND_TIMEOUT = doc["ZMQ_SEND_TIMEOUT"].GetUint64();
	ZEROMQ_SERVER_PATH = doc["ZEROMQ_SERVER_PATH"].GetString();
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
	FILE *fptr = fopen(dir.c_str(), "r");
	if (!fptr)
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
			return std::string(doc[value.c_str()].GetString());
		return stringify(doc[value.c_str()]);
	}
	spdlog::error("Can't find requested field at config {}", value);
	return "";
}

// GCOVR_EXCL_START
void alarmFunc(int)
{
	struct timespec ts;

	++alarmCtr;
	if (loopFlag)
		alarm(ALARM_INTERVAL);
}
// GCOVR_EXCL_STOP

// GCOVR_EXCL_START
void interruptFunc(int)
{
	if (loopFlag)
		loopFlag = false;
	else
		(void)!write(STDERR_FILENO, "Interrupt in progress...\n", 26);
}
// GCOVR_EXCL_STOP
