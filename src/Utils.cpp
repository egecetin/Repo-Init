#include "Utils.h"
#include "Sentry.h"
#include "Version.h"

#include <execinfo.h>
#include <signal.h>

#include <curl/curlver.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/writer.h>
#include <sentry.h>
#include <spdlog/spdlog.h>
#include <zmq.hpp>

#include <spdlog/sinks/dup_filter_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/syslog_sink.h>

uintmax_t ALARM_INTERVAL;
uintmax_t HEARTBEAT_INTERVAL;

int ZMQ_SEND_TIMEOUT;
int ZMQ_RECV_TIMEOUT;
std::string CONTROL_IPC_PATH;

uint16_t TELNET_PORT;
std::string PROMETHEUS_ADDR;

volatile time_t currentTime;
volatile uintmax_t alarmCtr;
volatile bool loopFlag;
volatile bool sigReadyFlag;

// GCOVR_EXCL_START
void print_version(void)
{
	int major = 0, minor = 0, patch = 0;
	spdlog::info("{}                                : v{}", PROJECT_NAME, PROJECT_FULL_REVISION);
	spdlog::info("  CppZMQ                          : v{}.{}.{}", CPPZMQ_VERSION_MAJOR, CPPZMQ_VERSION_MINOR,
				 CPPZMQ_VERSION_PATCH);
	spdlog::info("  Curl                            : v{}", LIBCURL_VERSION);
	spdlog::info("  Rapidjson                       : v{}", RAPIDJSON_VERSION_STRING);
	spdlog::info("  Sentry                          : v{}", SENTRY_SDK_VERSION);
	spdlog::info("  Spdlog                          : v{}.{}.{}", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);
	zmq_version(&major, &minor, &patch);
	spdlog::info("  ZeroMQ                          : v{}.{}.{}", major, minor, patch);
}
// GCOVR_EXCL_STOP

// GCOVR_EXCL_START
bool init_logger(int argc, char **argv)
{
	// Initial config
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [" + std::string(PROJECT_NAME) + "] [%^%l%$] : %v");
	print_version();

	// Prepare spdlog loggers
	auto dup_filter = std::make_shared<spdlog::sinks::dup_filter_sink_mt>(std::chrono::seconds(5));
	dup_filter->add_sink(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
	dup_filter->add_sink(std::make_shared<spdlog::sinks::rotating_file_sink_mt>("error.log", 1048576 * 5, 3, false));
	dup_filter->add_sink(std::make_shared<spdlog::sinks::syslog_sink_mt>(PROJECT_NAME, LOG_USER, 0, false));
	dup_filter->add_sink(
		std::make_shared<spdlog::sinks::sentry_api_sink_mt>(readSingleConfig(CONFIG_FILE_PATH, "SENTRY_ADDRESS")));

	// Register main logger
	auto combined_logger = std::make_shared<spdlog::logger>(PROJECT_NAME, dup_filter);
	spdlog::set_default_logger(combined_logger);
	spdlog::warn("{} started", PROJECT_NAME);

#ifdef NDEBUG
	spdlog::set_level(spdlog::level::warn);
#else
	spdlog::set_level(spdlog::level::info);
#endif

	// Parse input arguments
	InputParser input(argc, argv);
	if (input.cmdOptionExists("-v"))
		spdlog::set_level(spdlog::level::info);
	if (input.cmdOptionExists("-vv"))
		spdlog::set_level(spdlog::level::debug);
	if (input.cmdOptionExists("-vvv"))
		spdlog::set_level(spdlog::level::trace);

	return true;
}
// GCOVR_EXCL_STOP

// GCOVR_EXCL_START
void close_logger(void) { spdlog::shutdown(); }
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
	std::vector<std::string> list = {"ZMQ_RECV_TIMEOUT", "ZMQ_SEND_TIMEOUT", "CONTROL_IPC_PATH"};

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
