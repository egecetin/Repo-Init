#pragma once

#include <mutex>
#include <string>

#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>

namespace spdlog
{
	namespace sinks
	{
		template <typename Mutex> class sentry_api_sink : public base_sink<Mutex>
		{
		  public:
			explicit sentry_api_sink(const std::string &sentryAddress);
			~sentry_api_sink();

		  protected:
			void sink_it_(const details::log_msg &msg) override;
			void flush_() override;

		  private:
			bool sentryAvailable{false};
		};

		using sentry_api_sink_mt = sentry_api_sink<std::mutex>;
		using sentry_api_sink_st = sentry_api_sink<details::null_mutex>;
	} // namespace sinks
} // namespace spdlog
