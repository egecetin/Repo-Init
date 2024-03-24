#include "logging/Loki.hpp"

#include "Version.h"
#include "utils/FileHelpers.hpp"

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
			if (lokiAddress.empty())
			{
				return;
			}

			connHandler = std::make_unique<HTTP>(lokiAddress);

			// Pre-allocate buffers
			internalLogBuffer.push_back({"debug", std::vector<std::pair<std::string, std::string>>()});
			internalLogBuffer.push_back({"info", std::vector<std::pair<std::string, std::string>>()});
			internalLogBuffer.push_back({"warn", std::vector<std::pair<std::string, std::string>>()});
			internalLogBuffer.push_back({"error", std::vector<std::pair<std::string, std::string>>()});
			internalLogBuffer.push_back({"critical", std::vector<std::pair<std::string, std::string>>()});

			// Prepare information (Loki limits maximum number of labels with 15)
			basicInformation = "";
			basicInformation += std::string(R"("release_version":"v)") + PROJECT_FULL_REVISION + "\",";
			basicInformation +=
				std::string(R"("release_date":")") + PROJECT_BUILD_DATE + " " + PROJECT_BUILD_TIME + "\",";
			basicInformation += std::string(R"("compiler_name":")") + COMPILER_NAME + "\",";
			basicInformation += std::string(R"("compiler_version":")") + COMPILER_VERSION + "\",";
			basicInformation += std::string(R"("build":")") + BUILD_TYPE + "\",";

			// Parse hostname
			std::array<char, BUFSIZ> hostBuffer{};
			gethostname(hostBuffer.data(), BUFSIZ);
			basicInformation += std::string(R"("hostname":")") + hostBuffer.data() + "\",";

			// Parse CPU information
			const std::string cpuInfoPath = "/proc/cpuinfo";
			std::string word;

			findFromFile(cpuInfoPath, "^siblings", word);
			basicInformation += std::string(R"("cpu_threadcount":")") + word + "\",";
			findFromFile(cpuInfoPath, "^(cpu cores)", word);
			basicInformation += std::string(R"("cpu_corecount":")") + word + "\",";
			findFromFile(cpuInfoPath, "^(model name)", word);
			basicInformation += std::string(R"("cpu_model":")") + word + "\",";
			findFromFile(cpuInfoPath, "^vendor_id", word);
			basicInformation += std::string(R"("cpu_vendorid":")") + word + "\",";

			lokiAvailable = true;
		}

		template <typename Mutex> loki_api_sink<Mutex>::~loki_api_sink() = default;

		template <typename Mutex> void loki_api_sink<Mutex>::sink_it_(const details::log_msg &msg)
		{
			if (!lokiAvailable)
			{
				return;
			}

			if (msg.level >= spdlog::level::debug && msg.level <= spdlog::level::critical)
			{
				internalLogBuffer[static_cast<size_t>(msg.level) - 1].logs.push_back(
					{std::to_string(msg.time.time_since_epoch().count()),
					 std::string(msg.payload.data(), msg.payload.size())});
			}
		}

		template <typename Mutex> void loki_api_sink<Mutex>::flush_()
		{
			bool flag = false;

			// Prepare JSON
			std::ostringstream sStream;

			sStream << "{\"streams\":[";
			for (auto &entry : internalLogBuffer)
			{
				if (entry.logs.empty())
				{
					continue;
				}

				if (flag)
				{
					sStream << ",";
				}
				flag = true;
				sStream << "{\"stream\":{" << basicInformation + R"("level":")" << entry.level << R"("},"values":[)";

				bool subflag = false;
				for (const auto &subentry : entry.logs)
				{
					if (subflag)
					{
						sStream << ",";
					}
					subflag = true;
					sStream << "[\"" << subentry.first << "\",\"" << subentry.second << "\"]";
				}
				sStream << "]}";

				entry.logs.clear();
			}
			sStream << "]}";

			if (flag)
			{
				// Send request
				std::string recvData;
				HttpStatus::Code replyCode = HttpStatus::Code::xxx_max;
				connHandler->sendPOSTRequest("/loki/api/v1/push", sStream.str(), recvData, replyCode);
			}
		}

		template class loki_api_sink<std::mutex>;
		template class loki_api_sink<details::null_mutex>;
	} // namespace sinks
} // namespace spdlog
