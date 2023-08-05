#include "logging/Logger.hpp"
#include "test-static-definitions.h"

#include <chrono>
#include <future>
#include <thread>

#include <gtest/gtest.h>

TEST(Logger_UnitTests, Logging)
{
	std::future<int> shResult;

	const MainLogger logger(0, nullptr, TEST_CONFIG_PATH);

	// Launch script
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	shResult = std::async(std::launch::async, []() {
		return system(("python3 " + std::string(TEST_LOG_SERVER_PY_PATH) + " >/dev/null").c_str());
	});

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	spdlog::trace("Trace message");
	spdlog::debug("Debug message");
	spdlog::warn("Warning message");
	spdlog::error("Error message");
	spdlog::critical("Critical message");

	shResult.wait();
	ASSERT_EQ(0, shResult.get());
}
