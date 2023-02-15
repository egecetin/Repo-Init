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

MainLogger::MainLogger(int argc, char **argv, const std::string &configPath)
{
	spdlog::set_level(spdlog::level::off);

	// Prepare spdlog loggers
	auto dup_filter = std::make_shared<spdlog::sinks::dup_filter_sink_mt>(std::chrono::seconds(5));
	dup_filter->add_sink(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
	dup_filter->add_sink(std::make_shared<spdlog::sinks::rotating_file_sink_mt>("error.log", 1048576 * 5, 3, false));
	dup_filter->add_sink(std::make_shared<spdlog::sinks::syslog_sink_mt>(PROJECT_NAME, LOG_USER, 0, false));
	dup_filter->add_sink(
		std::make_shared<spdlog::sinks::loki_api_sink_mt>(readSingleConfig(configPath, "LOKI_ADDRESS")));
	dup_filter->add_sink(
		std::make_shared<spdlog::sinks::sentry_api_sink_mt>(readSingleConfig(configPath, "SENTRY_ADDRESS")));

	// Register main logger
	mainLogger = std::make_shared<spdlog::logger>(PROJECT_NAME, dup_filter);
	spdlog::set_default_logger(mainLogger);
	spdlog::flush_every(std::chrono::seconds(2));

#ifdef NDEBUG
	mainLogger->set_level(spdlog::level::warn);
#else
	mainLogger->set_level(spdlog::level::info);
#endif

	// Parse input arguments
	InputParser input(argc, argv);
	if (input.cmdOptionExists("-v"))
		mainLogger->set_level(spdlog::level::info);
	if (input.cmdOptionExists("-vv"))
		mainLogger->set_level(spdlog::level::debug);
	if (input.cmdOptionExists("-vvv"))
		mainLogger->set_level(spdlog::level::trace);
}

MainLogger::~MainLogger() { mainLogger->flush(); }
