#include "logging/Logger.hpp"
#include "logging/Loki.hpp"
#include "logging/Sentry.hpp"

#include "Utils.hpp"
#include "Version.h"

#include <spdlog/sinks/dup_filter_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/syslog_sink.h>
#include <spdlog/spdlog.h>

MainLogger::MainLogger(int argc, char **argv)
{
	// Initial config
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [" + std::string(PROJECT_NAME) + "] [%^%l%$] : %v");
	print_version();

	// Prepare spdlog loggers
	auto dup_filter = std::make_shared<spdlog::sinks::dup_filter_sink_mt>(std::chrono::seconds(5));
	dup_filter->add_sink(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
	dup_filter->add_sink(std::make_shared<spdlog::sinks::rotating_file_sink_mt>("error.log", 1048576 * 5, 3, false));
	dup_filter->add_sink(std::make_shared<spdlog::sinks::syslog_sink_mt>(PROJECT_NAME, LOG_USER, 0, false));
	dup_filter->add_sink(
		std::make_shared<spdlog::sinks::loki_api_sink_mt>(readSingleConfig(CONFIG_FILE_PATH, "LOKI_ADDRESS")));
	dup_filter->add_sink(
		std::make_shared<spdlog::sinks::sentry_api_sink_mt>(readSingleConfig(CONFIG_FILE_PATH, "SENTRY_ADDRESS")));

	// Register main logger
	auto combined_logger = std::make_shared<spdlog::logger>(PROJECT_NAME, dup_filter);
	spdlog::set_default_logger(combined_logger);
	spdlog::flush_every(std::chrono::seconds(2));
	spdlog::warn("{} started", PROJECT_NAME);

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
}

MainLogger::~MainLogger()
{
	spdlog::default_logger()->flush();
	spdlog::shutdown();
}