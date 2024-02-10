#include "logging/Sentry.hpp"
#include "Utils.hpp"
#include "Version.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <unistd.h>

#include <sentry.h>
#include <spdlog/spdlog.h>

// MAC address length for character string
constexpr int MAC_LEN = 18;

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

			// Context: Host
			std::array<char, BUFSIZ> hostBuffer{};
			gethostname(hostBuffer.data(), BUFSIZ);
			const sentry_value_t hostContext = sentry_value_new_object();
			sentry_value_set_by_key(hostContext, "Hostname", sentry_value_new_string(hostBuffer.data()));

			// Parse CPU information
			const std::string cpuInfoPath = "/proc/cpuinfo";
			std::string word;

			findFromFile(cpuInfoPath, "^siblings", word);
			sentry_value_set_by_key(hostContext, "Thread count", sentry_value_new_string(word.c_str()));
			findFromFile(cpuInfoPath, "^(cpu cores)", word);
			sentry_value_set_by_key(hostContext, "Core count", sentry_value_new_string(word.c_str()));
			findFromFile(cpuInfoPath, "^(model name)", word);
			sentry_value_set_by_key(hostContext, "Model", sentry_value_new_string(word.c_str()));
			findFromFile(cpuInfoPath, "^vendor_id", word);
			sentry_value_set_by_key(hostContext, "Vendor ID", sentry_value_new_string(word.c_str()));

			sentry_set_context("Host", hostContext);

			// Context: Network
			const sentry_value_t networkContext = sentry_value_new_object();

			ifaddrs *ifaddr = nullptr;
			if (getifaddrs(&ifaddr) != -1)
			{
				// Iterate interfaces
				for (ifaddrs *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
				{
					if (ifa->ifa_addr == nullptr)
					{
						continue;
					}

					switch (ifa->ifa_addr->sa_family)
					{
					case AF_INET:
						if (((ifa->ifa_flags & IFF_PROMISC) != 0) || ((ifa->ifa_flags & IFF_UP) != 0))
						{
							std::array<char, INET_ADDRSTRLEN> host{};
							inet_ntop(AF_INET, &(reinterpret_cast<sockaddr_in *>(ifa->ifa_addr))->sin_addr, host.data(),
									  INET_ADDRSTRLEN);
							sentry_value_set_by_key(networkContext, (std::string(ifa->ifa_name) + ".ipv4").c_str(),
													sentry_value_new_string(host.data()));
						}
						break;
					case AF_INET6:
						if (((ifa->ifa_flags & IFF_PROMISC) != 0) || ((ifa->ifa_flags & IFF_UP) != 0))
						{
							std::array<char, INET6_ADDRSTRLEN> host{};
							inet_ntop(AF_INET6, &(reinterpret_cast<sockaddr_in6 *>(ifa->ifa_addr))->sin6_addr,
									  host.data(), INET6_ADDRSTRLEN);
							sentry_value_set_by_key(networkContext, (std::string(ifa->ifa_name) + ".ipv6").c_str(),
													sentry_value_new_string(host.data()));
						}
						break;
					case AF_PACKET:
						if (((ifa->ifa_flags & IFF_PROMISC) != 0) || ((ifa->ifa_flags & IFF_UP) != 0))
						{
							std::array<char, MAC_LEN> host{};
							const auto *sock = reinterpret_cast<sockaddr_ll *>(ifa->ifa_addr);
							if (snprintf(host.data(), MAC_LEN, "%02x:%02x:%02x:%02x:%02x:%02x", sock->sll_addr[0],
										 sock->sll_addr[1], sock->sll_addr[2], sock->sll_addr[3], sock->sll_addr[4],
										 sock->sll_addr[5]) > 0)
							{
								sentry_value_set_by_key(networkContext, (std::string(ifa->ifa_name) + ".mac").c_str(),
														sentry_value_new_string(host.data()));
							}
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

		template class sentry_api_sink<std::mutex>;
		template class sentry_api_sink<details::null_mutex>;
	} // namespace sinks
} // namespace spdlog
