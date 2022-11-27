#include "logging/Loki.hpp"
#include "Version.h"

#include <sstream>

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <unistd.h>

#include <spdlog/spdlog.h>

namespace spdlog
{
	namespace sinks
	{
		template <typename Mutex> loki_api_sink<Mutex>::loki_api_sink(const std::string &lokiAddress)
		{
			lokiAvailable = false;
			if (lokiAddress.empty())
				return;

			connHandler = std::make_unique<HTTP>(lokiAddress);

			// Pre-allocate buffers
			internalLogBuffer.push_back({"debug", std::vector<std::pair<std::string, std::string>>()});
			internalLogBuffer.push_back({"info", std::vector<std::pair<std::string, std::string>>()});
			internalLogBuffer.push_back({"warn", std::vector<std::pair<std::string, std::string>>()});
			internalLogBuffer.push_back({"error", std::vector<std::pair<std::string, std::string>>()});
			internalLogBuffer.push_back({"critical", std::vector<std::pair<std::string, std::string>>()});

			// Prepare information (Loki limits maximum number of labels with 15)
			basicInformation = "";
			basicInformation += std::string("\"release_version\":\"v") + PROJECT_FULL_REVISION + "\",";
			basicInformation +=
				std::string("\"release_date\":\"") + PROJECT_BUILD_DATE + " " + PROJECT_BUILD_TIME + "\",";
			basicInformation += std::string("\"compiler_name\":\"") + COMPILER_NAME + "\",";
			basicInformation += std::string("\"compiler_version\":\"") + COMPILER_VERSION + "\",";
			basicInformation += std::string("\"build\":\"") + BUILD_TYPE + "\",";

			char hostBuffer[BUFSIZ];
			gethostname(hostBuffer, BUFSIZ);
			basicInformation += std::string("\"hostname\":\"") + hostBuffer + "\",";

			FILE *cpu_info = fopen("/proc/cpuinfo", "r");
			unsigned int thread_count, core_count;
			while (!fscanf(cpu_info, "siblings\t: %u", &thread_count))
				(void)!fscanf(cpu_info, "%*[^s]");
			basicInformation += std::string("\"cpu_threadcount\":\"") + std::to_string(thread_count) + "\",";
			rewind(cpu_info);
			while (!fscanf(cpu_info, "cpu cores\t: %u", &core_count))
				(void)!fscanf(cpu_info, "%*[^c]");
			basicInformation += std::string("\"cpu_corecount\":\"") + std::to_string(core_count) + "\",";
			rewind(cpu_info);
			while (!fscanf(cpu_info, "model name\t: %8191[^\n]", hostBuffer))
				(void)!fscanf(cpu_info, "%*[^m]");
			basicInformation += std::string("\"cpu_model\":\"") + hostBuffer + "\",";
			rewind(cpu_info);
			while (!fscanf(cpu_info, "vendor_id\t: %8191s", hostBuffer))
				(void)!fscanf(cpu_info, "%*[^v]");
			basicInformation += std::string("\"cpu_vendorid\":\"") + hostBuffer + "\",";
			fclose(cpu_info);

			lokiAvailable = true;
		}

		template <typename Mutex> loki_api_sink<Mutex>::~loki_api_sink() {}

		template <typename Mutex> void loki_api_sink<Mutex>::sink_it_(const details::log_msg &msg)
		{
			if (!lokiAvailable)
				return;
			if (msg.level >= spdlog::level::debug && msg.level <= spdlog::level::critical)
				internalLogBuffer[static_cast<int>(msg.level) - 1].logs.push_back(
					{std::to_string(msg.time.time_since_epoch().count()),
					 std::string(msg.payload.data(), msg.payload.size())});
		}

		template <typename Mutex> void loki_api_sink<Mutex>::flush_()
		{
			bool flag = false;

			// Prepare JSON
			std::ostringstream ss;

			ss << "{\"streams\":[";
			for (auto &entry : internalLogBuffer)
			{
				if (entry.logs.empty())
					continue;

				if (flag)
					ss << ",";
				flag = true;
				ss << "{\"stream\":{" << basicInformation + "\"level\":\"" << entry.level << "\"},\"values\":[";

				bool subflag = false;
				for (const auto &subentry : entry.logs)
				{
					if (subflag)
						ss << ",";
					subflag = true;
					ss << "[\"" << subentry.first << "\",\"" << subentry.second << "\"]";
				}
				ss << "]}";

				entry.logs.clear();
			}
			ss << "]}";

			if (flag)
			{
				// Send request
				std::string recvData;
				HttpStatus::Code replyCode = HttpStatus::Code::xxx_max;
				connHandler->sendPOSTRequest("/loki/api/v1/push", ss.str(), recvData, replyCode);
			}
		}

		template class SPDLOG_API loki_api_sink<std::mutex>;
		template class SPDLOG_API loki_api_sink<details::null_mutex>;
	} // namespace sinks
} // namespace spdlog