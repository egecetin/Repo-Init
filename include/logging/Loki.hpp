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
		/**
		 * @brief A custom sink for spdlog that sends log messages to a Loki server.
		 *
		 * The loki_api_sink class is a custom sink for spdlog that sends log messages to a Loki server.
		 * It provides functionality to send log messages to a specified Loki server address.
		 * The log messages are sent using HTTP requests.
		 *
		 * @tparam Mutex The type of mutex to use for thread-safety.
		 */
		template <typename Mutex> class loki_api_sink : public base_sink<Mutex> {
		  public:
			/**
			 * @brief Constructs a loki_api_sink object with the specified Loki server address.
			 *
			 * @param lokiAddress The address of the Loki server to send log messages to.
			 */
			explicit loki_api_sink(const std::string &lokiAddress);

			/**
			 * @brief Destroys the loki_api_sink object.
			 */
			~loki_api_sink();

		  protected:
			/**
			 * @brief Sends the log message to the Loki server.
			 *
			 * This function is called by spdlog to send the log message to the Loki server.
			 * It is overridden from the base_sink class.
			 *
			 * @param msg The log message to be sent.
			 */
			void sink_it_(const details::log_msg &msg) override;

			/**
			 * @brief Flushes any buffered log messages.
			 *
			 * This function is called by spdlog to flush any buffered log messages.
			 * It is overridden from the base_sink class.
			 */
			void flush_() override;

		  private:
			bool lokiAvailable{false};
			std::unique_ptr<HTTP> connHandler;
			std::string basicInformation;

			struct logInfo_t {
				std::string level;
				std::vector<std::pair<std::string, std::string>> logs;
			};
			std::vector<struct logInfo_t> internalLogBuffer;
		};

		using loki_api_sink_mt = loki_api_sink<std::mutex>;
		using loki_api_sink_st = loki_api_sink<details::null_mutex>;
	} // namespace sinks
} // namespace spdlog
