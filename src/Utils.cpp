#include "Utils.h"

#include "XXX_Version.h"

#include <arpa/inet.h>
#include <execinfo.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <netpacket/packet.h>
#include <signal.h>
#include <sys/socket.h>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/writer.h>
#include <sentry.h>
#include <spdlog/spdlog.h>
#include <zmq.hpp>

uintmax_t ALARM_INTERVAL;

int ZMQ_SEND_TIMEOUT;
int ZMQ_RECV_TIMEOUT;
uint16_t TELNET_PORT;
std::string CONTROL_IPC_PATH;
std::string SENTRY_ADDRESS;

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
	spdlog::info("  CppZMQ                          : v{}.{}.{}", CPPZMQ_VERSION_MAJOR, CPPZMQ_VERSION_MINOR,
				 CPPZMQ_VERSION_PATCH);
	spdlog::info("  Sentry                          : v{}", SENTRY_SDK_VERSION);
}
// GCOVR_EXCL_STOP

// GCOVR_EXCL_START
void sentry_logger_spdlog(sentry_level_t level, const char *message, va_list args, void *)
{
	// Process format
	char buf[BUFSIZ];
	vsprintf(buf, message, args);

	// Write to spdlog
	switch (level)
	{
	case SENTRY_LEVEL_DEBUG:
		spdlog::debug(buf);
		break;
	case SENTRY_LEVEL_INFO:
		spdlog::info(buf);
		break;
	case SENTRY_LEVEL_WARNING:
		spdlog::warn(buf);
		break;
	case SENTRY_LEVEL_ERROR:
		spdlog::error(buf);
		break;
	case SENTRY_LEVEL_FATAL:
		spdlog::critical(buf);
		break;
	default:
		spdlog::warn("Unknown Sentry log level {}", static_cast<int>(level));
		break;
	}
}
// GCOVR_EXCL_STOP

bool prepare_sentry(void)
{
	if (SENTRY_ADDRESS.empty())
	{
		spdlog::warn("Sentry address is empty");
		return false;
	}

	// Set options
	sentry_options_t *sentryOptions = sentry_options_new();
	sentry_options_set_dsn(sentryOptions, SENTRY_ADDRESS.c_str());
	sentry_options_set_logger(sentryOptions, sentry_logger_spdlog, nullptr);

	// Init
	sentry_init(sentryOptions);

	// Tags
	sentry_set_tag("compiler.name", COMPILER_NAME);
	sentry_set_tag("compiler.version", COMPILER_VERSION);
	sentry_set_tag("build", BUILD_TYPE);

	// Context: Version
	std::string versionBuffer;
	int major = 0, minor = 0, patch = 0;
	sentry_value_t versionContext = sentry_value_new_object();
	versionBuffer = "v" + std::string(PROJECT_FULL_REVISION);
	sentry_value_set_by_key(versionContext, "XXX", sentry_value_new_string(versionBuffer.c_str()));
	versionBuffer = "v" + std::to_string(SPDLOG_VER_MAJOR) + "." + std::to_string(SPDLOG_VER_MINOR) + "." +
					std::to_string(SPDLOG_VER_PATCH);
	sentry_value_set_by_key(versionContext, "Spdlog", sentry_value_new_string(versionBuffer.c_str()));
	versionBuffer = "v" + std::string(RAPIDJSON_VERSION_STRING);
	sentry_value_set_by_key(versionContext, "Rapidjson", sentry_value_new_string(versionBuffer.c_str()));
	zmq_version(&major, &minor, &patch);
	versionBuffer = "v" + std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
	sentry_value_set_by_key(versionContext, "ZeroMQ", sentry_value_new_string(versionBuffer.c_str()));
	versionBuffer = "v" + std::to_string(CPPZMQ_VERSION_MAJOR) + "." + std::to_string(CPPZMQ_VERSION_MINOR) + "." +
					std::to_string(CPPZMQ_VERSION_PATCH);
	sentry_value_set_by_key(versionContext, "CppZMQ", sentry_value_new_string(versionBuffer.c_str()));
	// Sentry send its version on default
	sentry_set_context("Version", versionContext);

	// Context: Host
	char hostBuffer[BUFSIZ];
	sentry_value_t hostContext = sentry_value_new_object();
	gethostname(hostBuffer, BUFSIZ);
	sentry_value_set_by_key(hostContext, "Hostname", sentry_value_new_string(hostBuffer));

	FILE *cpu_info = fopen("/proc/cpuinfo", "r");
	unsigned int thread_count, core_count;
	while (!fscanf(cpu_info, "siblings\t: %u", &thread_count))
		(void)!fscanf(cpu_info, "%*[^s]");
	sentry_value_set_by_key(hostContext, "Thread count", sentry_value_new_int32(thread_count));
	rewind(cpu_info);
	while (!fscanf(cpu_info, "cpu cores\t: %u", &core_count))
		(void)!fscanf(cpu_info, "%*[^c]");
	sentry_value_set_by_key(hostContext, "Core count", sentry_value_new_int32(core_count));
	rewind(cpu_info);
	while (!fscanf(cpu_info, "model name\t: %8191[^\n]", hostBuffer))
		(void)!fscanf(cpu_info, "%*[^m]");
	sentry_value_set_by_key(hostContext, "Model", sentry_value_new_string(hostBuffer));
	rewind(cpu_info);
	while (!fscanf(cpu_info, "vendor_id\t: %s", hostBuffer))
		(void)!fscanf(cpu_info, "%*[^v]");
	sentry_value_set_by_key(hostContext, "Vendor ID", sentry_value_new_string(hostBuffer));
	fclose(cpu_info);

	sentry_set_context("Host", hostContext);

	// Context: Network
	sentry_value_t networkContext = sentry_value_new_object();

	struct ifaddrs *ifaddr, *ifa;
	char host[INET6_ADDRSTRLEN];
	if (getifaddrs(&ifaddr) != -1)
	{
		// Iterate interfaces
		for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
		{
			if (ifa->ifa_addr == NULL)
				continue;

			switch (ifa->ifa_addr->sa_family)
			{
			case AF_INET:
				if ((ifa->ifa_flags & IFF_PROMISC) || (ifa->ifa_flags & IFF_UP))
				{
					inet_ntop(AF_INET, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, host, INET6_ADDRSTRLEN);
					sentry_value_set_by_key(networkContext, (std::string(ifa->ifa_name) + ".ipv4").c_str(),
											sentry_value_new_string(host));
				}
				break;
			case AF_INET6:
				if ((ifa->ifa_flags & IFF_PROMISC) || (ifa->ifa_flags & IFF_UP))
				{
					inet_ntop(AF_INET6, &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr, host, INET6_ADDRSTRLEN);
					sentry_value_set_by_key(networkContext, (std::string(ifa->ifa_name) + ".ipv6").c_str(),
											sentry_value_new_string(host));
				}
				break;
			case AF_PACKET:
				if ((ifa->ifa_flags & IFF_PROMISC) || (ifa->ifa_flags & IFF_UP))
				{
					struct sockaddr_ll *s = (struct sockaddr_ll *)(ifa->ifa_addr);
					sprintf(host, "%02x:%02x:%02x:%02x:%02x:%02x", s->sll_addr[0], s->sll_addr[1], s->sll_addr[2],
							s->sll_addr[3], s->sll_addr[4], s->sll_addr[5]);
					sentry_value_set_by_key(networkContext, (std::string(ifa->ifa_name) + ".mac").c_str(),
											sentry_value_new_string(host));
				}
				break;
			default:
				break;
			}
		}
		freeifaddrs(ifaddr);
	}

	sentry_set_context("Network", networkContext);

	return true;
}

void closeSentry() { sentry_close(); }

bool init_logger(int argc, char **argv)
{
	// Initial config
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [XXX] [%^%l%$] : %v");
	print_version();

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

	// Prepare spdlog loggers

	// Prepare Sentry API
	SENTRY_ADDRESS = readSingleConfig(CONFIG_FILE_PATH, "SENTRY_ADDRESS");
	if (!prepare_sentry())
		spdlog::warn("Can't init Sentry");
}

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
	std::vector<std::string> list = {"ZMQ_RECV_TIMEOUT", "ZMQ_SEND_TIMEOUT", "TELNET_PORT", "CONTROL_IPC_PATH",
									 "SENTRY_ADDRESS"};

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
	SENTRY_ADDRESS = doc["SENTRY_ADDRESS"].GetString();

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
