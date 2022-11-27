#pragma once

#include "connection/Http.hpp"

#include <mutex>
#include <string>
#include <vector>

#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>

namespace spdlog
{
	namespace sinks
	{
		template <typename Mutex> class loki_api_sink : public base_sink<Mutex>
		{
		  public:
			explicit loki_api_sink(const std::string &lokiAddress);
			~loki_api_sink();

		  protected:
			void sink_it_(const details::log_msg &msg) override;
			void flush_() override;

		  private:
			bool lokiAvailable;
			std::unique_ptr<HTTP> connHandler;
			std::string basicInformation;

			struct logInfo_t
			{
				std::string level;
				std::vector<std::pair<std::string, std::string>> logs;
			};
			std::vector<struct logInfo_t> internalLogBuffer;
		};

		using loki_api_sink_mt = loki_api_sink<std::mutex>;
		using loki_api_sink_st = loki_api_sink<details::null_mutex>;
	} // namespace sinks
} // namespace spdlog
