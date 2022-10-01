#include "logging/Sentry.hpp"
#include "Version.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <netpacket/packet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <curl/curlver.h>
#include <rapidjson/rapidjson.h>
#include <sentry.h>
#include <spdlog/spdlog.h>
#include <zmq.hpp>

// GCOVR_EXCL_START
namespace spdlog
{
	namespace sinks
	{
		template <typename Mutex> sentry_api_sink<Mutex>::sentry_api_sink(const std::string &sentryAddress)
		{
			sentryAvailable = false;
			if (sentryAddress.empty())
				return;
			sentryAvailable = true;

			// Set options
			sentry_options_t *sentryOptions = sentry_options_new();
			sentry_options_set_release(sentryOptions, PROJECT_FULL_REVISION);
			sentry_options_set_dsn(sentryOptions, sentryAddress.c_str());

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
			sentry_value_set_by_key(versionContext, PROJECT_NAME, sentry_value_new_string(versionBuffer.c_str()));

			/* ############################# MAKE MODIFICATIONS HERE ############################# */
			versionBuffer = "v" + std::to_string(CPPZMQ_VERSION_MAJOR) + "." + std::to_string(CPPZMQ_VERSION_MINOR) +
							"." + std::to_string(CPPZMQ_VERSION_PATCH);
			sentry_value_set_by_key(versionContext, "CppZMQ", sentry_value_new_string(versionBuffer.c_str()));
			versionBuffer = "v" + std::string(LIBCURL_VERSION);
			sentry_value_set_by_key(versionContext, "Curl", sentry_value_new_string(versionBuffer.c_str()));
			versionBuffer = "v" + std::string(RAPIDJSON_VERSION_STRING);
			sentry_value_set_by_key(versionContext, "Rapidjson", sentry_value_new_string(versionBuffer.c_str()));
			versionBuffer = "v" + std::string(SENTRY_SDK_VERSION);
			sentry_value_set_by_key(versionContext, "Sentry", sentry_value_new_string(versionBuffer.c_str()));
			versionBuffer = "v" + std::to_string(SPDLOG_VER_MAJOR) + "." + std::to_string(SPDLOG_VER_MINOR) + "." +
							std::to_string(SPDLOG_VER_PATCH);
			sentry_value_set_by_key(versionContext, "Spdlog", sentry_value_new_string(versionBuffer.c_str()));
			zmq_version(&major, &minor, &patch);
			versionBuffer = "v" + std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
			sentry_value_set_by_key(versionContext, "ZeroMQ", sentry_value_new_string(versionBuffer.c_str()));

			/* ################################ END MODIFICATIONS ################################ */

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
							inet_ntop(AF_INET, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, host,
									  INET6_ADDRSTRLEN);
							sentry_value_set_by_key(networkContext, (std::string(ifa->ifa_name) + ".ipv4").c_str(),
													sentry_value_new_string(host));
						}
						break;
					case AF_INET6:
						if ((ifa->ifa_flags & IFF_PROMISC) || (ifa->ifa_flags & IFF_UP))
						{
							inet_ntop(AF_INET6, &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr, host,
									  INET6_ADDRSTRLEN);
							sentry_value_set_by_key(networkContext, (std::string(ifa->ifa_name) + ".ipv6").c_str(),
													sentry_value_new_string(host));
						}
						break;
					case AF_PACKET:
						if ((ifa->ifa_flags & IFF_PROMISC) || (ifa->ifa_flags & IFF_UP))
						{
							struct sockaddr_ll *s = (struct sockaddr_ll *)(ifa->ifa_addr);
							sprintf(host, "%02x:%02x:%02x:%02x:%02x:%02x", s->sll_addr[0], s->sll_addr[1],
									s->sll_addr[2], s->sll_addr[3], s->sll_addr[4], s->sll_addr[5]);
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
		}

		template <typename Mutex> sentry_api_sink<Mutex>::~sentry_api_sink() { sentry_close(); }

		template <typename Mutex> void sentry_api_sink<Mutex>::sink_it_(const details::log_msg &msg)
		{
			if (!sentryAvailable)
				return;
			switch (msg.level)
			{
			case spdlog::level::warn:
				sentry_capture_event(sentry_value_new_message_event(SENTRY_LEVEL_WARNING, "main", msg.payload.data()));
				break;
			case spdlog::level::err:
				sentry_capture_event(sentry_value_new_message_event(SENTRY_LEVEL_ERROR, "main", msg.payload.data()));
				break;
			case spdlog::level::critical:
				sentry_capture_event(sentry_value_new_message_event(SENTRY_LEVEL_FATAL, "main", msg.payload.data()));
				break;
			case spdlog::level::debug:
			case spdlog::level::info:
			case spdlog::level::off:
			case spdlog::level::trace:
			default:
				break;
			}
		}

		template <typename Mutex> void sentry_api_sink<Mutex>::flush_() {}

		template class SPDLOG_API sentry_api_sink<std::mutex>;
		template class SPDLOG_API sentry_api_sink<details::null_mutex>;
	} // namespace sinks
} // namespace spdlog
// GCOVR_EXCL_STOP
