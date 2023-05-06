#include "logging/Sentry.hpp"
#include "Version.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <unistd.h>

#include <sentry.h>
#include <spdlog/spdlog.h>

// GCOVR_EXCL_START

namespace spdlog
{
	namespace sinks
	{
		template <typename Mutex> sentry_api_sink<Mutex>::sentry_api_sink(const std::string &sentryAddress)
		{
			if (sentryAddress.empty())
			{
				return;
			}
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
			const sentry_value_t versionContext = sentry_value_new_object();
			versionBuffer = "v" + std::string(PROJECT_FULL_REVISION);
			sentry_value_set_by_key(versionContext, PROJECT_NAME, sentry_value_new_string(versionBuffer.c_str()));
			versionBuffer = std::string(PROJECT_BUILD_DATE) + " " + PROJECT_BUILD_TIME;
			sentry_value_set_by_key(versionContext, "Release Date", sentry_value_new_string(versionBuffer.c_str()));
			/* ################################################################################### */
			/* ############################# MAKE MODIFICATIONS HERE ############################# */
			/* ################################################################################### */

			/* ################################################################################### */
			/* ################################ END MODIFICATIONS ################################ */
			/* ################################################################################### */

			sentry_set_context("Version", versionContext);

			// NOLINTBEGIN
			// Context: Host
			char hostBuffer[BUFSIZ];
			gethostname(hostBuffer, BUFSIZ);
			const sentry_value_t hostContext = sentry_value_new_object();
			sentry_value_set_by_key(hostContext, "Hostname", sentry_value_new_string(hostBuffer));

			FILE *cpu_info = fopen("/proc/cpuinfo", "r");
			unsigned int core_count;
			unsigned int thread_count;
			while (fscanf(cpu_info, "siblings\t: %u", &thread_count) == 0)
			{
				(void)!fscanf(cpu_info, "%*[^s]");
			}
			sentry_value_set_by_key(hostContext, "Thread count", sentry_value_new_int32(thread_count));
			rewind(cpu_info);

			while (fscanf(cpu_info, "cpu cores\t: %u", &core_count) == 0)
			{
				(void)!fscanf(cpu_info, "%*[^c]");
			}
			sentry_value_set_by_key(hostContext, "Core count", sentry_value_new_int32(core_count));
			rewind(cpu_info);

			while (fscanf(cpu_info, "model name\t: %8191[^\n]", hostBuffer) == 0)
			{
				(void)!fscanf(cpu_info, "%*[^m]");
			}
			sentry_value_set_by_key(hostContext, "Model", sentry_value_new_string(hostBuffer));
			rewind(cpu_info);

			while (fscanf(cpu_info, "vendor_id\t: %8191s", hostBuffer) == 0)
			{
				(void)!fscanf(cpu_info, "%*[^v]");
			}
			sentry_value_set_by_key(hostContext, "Vendor ID", sentry_value_new_string(hostBuffer));
			fclose(cpu_info);

			sentry_set_context("Host", hostContext);

			// Context: Network
			const sentry_value_t networkContext = sentry_value_new_object();

			struct ifaddrs *ifaddr = nullptr;
			struct ifaddrs *ifa = nullptr;
			if (getifaddrs(&ifaddr) != -1)
			{
				// Iterate interfaces
				for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
				{
					if (ifa->ifa_addr == nullptr)
					{
						continue;
					}

					switch (ifa->ifa_addr->sa_family)
					{
					case AF_INET:
						if (((ifa->ifa_flags & IFF_PROMISC) != 0U) || ((ifa->ifa_flags & IFF_UP) != 0U))
						{
							char host[INET_ADDRSTRLEN];
							inet_ntop(AF_INET, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, host, INET_ADDRSTRLEN);
							sentry_value_set_by_key(networkContext, (std::string(ifa->ifa_name) + ".ipv4").c_str(),
													sentry_value_new_string(host));
						}
						break;
					case AF_INET6:
						if (((ifa->ifa_flags & IFF_PROMISC) != 0U) || ((ifa->ifa_flags & IFF_UP) != 0U))
						{
							char host[INET6_ADDRSTRLEN];
							inet_ntop(AF_INET6, &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr, host,
									  INET6_ADDRSTRLEN);
							sentry_value_set_by_key(networkContext, (std::string(ifa->ifa_name) + ".ipv6").c_str(),
													sentry_value_new_string(host));
						}
						break;
					case AF_PACKET:
						if (((ifa->ifa_flags & IFF_PROMISC) != 0U) || ((ifa->ifa_flags & IFF_UP) != 0U))
						{
							char host[18];
							auto *s = (struct sockaddr_ll *)(ifa->ifa_addr);
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
			// NOLINTEND
		}

		template <typename Mutex> sentry_api_sink<Mutex>::~sentry_api_sink() { sentry_close(); }

		template <typename Mutex> void sentry_api_sink<Mutex>::sink_it_(const details::log_msg &msg)
		{
			if (!sentryAvailable)
			{
				return;
			}
			switch (msg.level)
			{
			case spdlog::level::warn:
				sentry_capture_event(sentry_value_new_message_event(
					SENTRY_LEVEL_WARNING, "main", std::string(msg.payload.data(), msg.payload.size()).c_str()));
				break;
			case spdlog::level::err:
				sentry_capture_event(sentry_value_new_message_event(
					SENTRY_LEVEL_ERROR, "main", std::string(msg.payload.data(), msg.payload.size()).c_str()));
				break;
			case spdlog::level::critical:
				sentry_capture_event(sentry_value_new_message_event(
					SENTRY_LEVEL_FATAL, "main", std::string(msg.payload.data(), msg.payload.size()).c_str()));
				break;
			case spdlog::level::trace:
			case spdlog::level::debug:
			case spdlog::level::info:
			case spdlog::level::off:
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
