#include "Utils.h"

#include "XXX_Version.h"

#include <execinfo.h>
#include <signal.h>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/writer.h>
#include <spdlog/spdlog.h>
#include <zmq.hpp>

uintmax_t ALARM_INTERVAL;

int ZMQ_SEND_TIMEOUT;
int ZMQ_RECV_TIMEOUT;
uint16_t TELNET_PORT;
std::string CONTROL_IPC_PATH;

volatile time_t currentTime;
volatile uintmax_t alarmCtr;
volatile bool loopFlag;
volatile bool sigReadyFlag;

// GCOVR_EXCL_START
void print_version(void)
{
	int major = 0, minor = 0, patch = 0;
	spdlog::info("XXX                               : v{}", PROJECT_FULL_REVISION);
	spdlog::info("  Spdlog                          : v{}.{}.{}", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);
	spdlog::info("  Rapidjson                       : v{}", RAPIDJSON_VERSION_STRING);
	zmq_version(&major, &minor, &patch);
	spdlog::info("  ZeroMQ                          : v{}.{}.{}", major, minor, patch);
	spdlog::info("  CPPZMQ                          : v{}.{}.{}", CPPZMQ_VERSION_MAJOR, CPPZMQ_VERSION_MINOR,
				 CPPZMQ_VERSION_PATCH);
}
// GCOVR_EXCL_STOP

template <typename T> std::string stringify(const T &o)
{
	rapidjson::StringBuffer sb;
	rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
	o.Accept(writer);
	return sb.GetString();
}

bool readConfig(const char *dir)
{
	FILE *fptr = fopen(dir, "r");
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
	std::vector<std::string> list = {
		"ZMQ_RECV_TIMEOUT",
		"ZMQ_SEND_TIMEOUT",
		"TELNET_PORT",
		"CONTROL_IPC_PATH",
	};

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
	TELNET_PORT = doc["TELNET_PORT"].GetUint();

	CONTROL_IPC_PATH = doc["CONTROL_IPC_PATH"].GetString();

	// Cleanup
	doc.GetAllocator().Clear();
	doc.RemoveAllMembers();

	return true;
}

std::string readSingleConfig(const char *dir, std::string value)
{
	FILE *fptr = fopen(dir, "r");
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

	// Get time
	clock_gettime(CLOCK_TAI, &ts);
	currentTime = ts.tv_sec;

	if (!sigReadyFlag)
		sigReadyFlag = true;
	if (loopFlag)
		alarm(ALARM_INTERVAL);
}
// GCOVR_EXCL_STOP

// GCOVR_EXCL_START
void backtracer(int)
{
	if (loopFlag)
	{
		// Give chance to break loops
		loopFlag = false;
		sleep(2);

		// Trace error
		void *array[100];
		int size;

		// get void*'s for all entries on the stack
		size = backtrace(array, 100);

		// print strings for all entries
		(void)!write(STDERR_FILENO, "Error signal\n", 14);
		backtrace_symbols_fd(array, size, STDERR_FILENO);
		(void)!write(STDERR_FILENO, "Error printed\n", 15);

		sleep(3);

		(void)!write(STDERR_FILENO, "Aborting\n", 9);
		abort();
	}
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
