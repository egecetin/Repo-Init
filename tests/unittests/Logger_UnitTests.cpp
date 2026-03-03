#include "logging/Logger.hpp"

#include "EchoServer.hpp"
#include "test-static-definitions.h"

#include <chrono>
#include <future>
#include <thread>

#include <gtest/gtest.h>

TEST(Logger_Tests, LoggingUnitTests)
{
	const MainLogger logger("http://localhost:8400",
							"http://username:password@localhost:8400/foo"); // pragma: allowlist secret

	ASSERT_NE(logger.getLogger(), nullptr);

	// Launch script
	EchoServer server(8400);

	ASSERT_NO_THROW(spdlog::trace("Trace message"));
	ASSERT_NO_THROW(spdlog::debug("Debug message"));
	ASSERT_NO_THROW(spdlog::warn("Warning message"));
	ASSERT_NO_THROW(spdlog::error("Error message"));
	ASSERT_NO_THROW(spdlog::critical("Critical message"));

	const MainLogger logger2("", "");
	ASSERT_NE(logger2.getLogger(), nullptr);

	ASSERT_NO_THROW(spdlog::trace("Trace message"));
	ASSERT_NO_THROW(spdlog::debug("Debug message"));
	ASSERT_NO_THROW(spdlog::warn("Warning message"));
	ASSERT_NO_THROW(spdlog::error("Error message"));
	ASSERT_NO_THROW(spdlog::critical("Critical message"));
}
