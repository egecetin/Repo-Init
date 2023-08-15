#include "Utils.hpp"
#include "metrics/PrometheusServer.hpp"
#include "test-static-definitions.h"
#include "zeromq/ZeroMQServer.hpp"

#include <chrono>
#include <future>
#include <thread>

#include <gtest/gtest.h>

TEST(ZeroMQ_Tests, ZeroMQServerUnitTests)
{
	std::string zeromqServerAddr = "tcp://127.0.0.1:8300";
	std::string promServerAddr = "localhost:8301";

	std::future<int> shResult;

	// For internal statistics
	PrometheusServer reporter(promServerAddr);

	// Init ZeroMQ Server
	auto zeromqServerPtr = std::make_shared<ZeroMQServer>();
	ASSERT_TRUE(zeromqServerPtr->initialise(zeromqServerAddr));
	ASSERT_FALSE(zeromqServerPtr->initialise(zeromqServerAddr));
	zeromqServerPtr->shutdown();

	ASSERT_TRUE(zeromqServerPtr->initialise(zeromqServerAddr, reporter.createNewRegistry()));
	zeromqServerPtr->messageCallback(ZeroMQServerMessageCallback);

	// Launch script
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	shResult = std::async(std::launch::async, []() {
		return system(("python3 " + std::string(TEST_ZEROMQ_PY_PATH) + " >/dev/null").c_str());
	});

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	for (size_t idx = 0; idx < 10; ++idx)
	{
		zeromqServerPtr->update();
	}

	shResult.wait();
	zeromqServerPtr->shutdown();
	zeromqServerPtr->shutdown();
	ASSERT_EQ(0, shResult.get());
}
