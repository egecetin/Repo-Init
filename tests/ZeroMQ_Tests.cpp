#include "Utils.hpp"
#include "metrics/Reporter.hpp"
#include "test-static-definitions.h"
#include "zeromq/ZeroMQServer.hpp"

#include <chrono>
#include <future>
#include <thread>

#include <gtest/gtest.h>

TEST(ZeroMQ_Tests, ZeroMQServerTests)
{
	std::future<int> shResult;

	// For internal statistics
	Reporter reporter(TEST_PROMETHEUS_SERVER_ADDR_3);

	// Init ZeroMQ Server
	auto zeromqServerPtr = std::make_shared<ZeroMQServer>();
	ASSERT_TRUE(zeromqServerPtr->initialise(TEST_ZMQ_SERVER_PATH));
	ASSERT_FALSE(zeromqServerPtr->initialise(TEST_ZMQ_SERVER_PATH));
	zeromqServerPtr->shutdown();

	ASSERT_TRUE(zeromqServerPtr->initialise(TEST_ZMQ_SERVER_PATH, reporter.getRegistry()));
	zeromqServerPtr->messageCallback(ZeroMQServerMessageCallback);

	// Launch script
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	shResult = std::async(std::launch::async, []() {
		return system(("python3 " + std::string(TEST_ZEROMQ_PY_PATH) + " >/dev/null").c_str());
	});

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	for (size_t idx = 0; idx < 50; ++idx)
	{
		zeromqServerPtr->update();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	shResult.wait();
	zeromqServerPtr->shutdown();
	ASSERT_EQ(0, shResult.get());
}
