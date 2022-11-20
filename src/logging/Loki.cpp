#include "logging/Loki.hpp"
#include "Version.h"

#include <sstream>

#include <spdlog/spdlog.h>

// GCOVR_EXCL_START
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
			std::stringstream ss;

			ss << "{\"streams\":[";
			for (auto &entry : internalLogBuffer)
			{
				if (entry.logs.empty())
					continue;

				if (flag)
					ss << ",";
				flag = true;
				ss << "{\"stream\":{\"level\":\"" << entry.level << "\"},\"values\":[";

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
// GCOVR_EXCL_STOP
