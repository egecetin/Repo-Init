#include "Utils.hpp"
#include "metrics/PrometheusServer.hpp"
#include "telnet/TelnetServer.hpp"
#include "test-static-definitions.h"

#include <chrono>
#include <future>
#include <thread>

#include <gtest/gtest.h>

TEST(Telnet_Tests, TelnetServerUnitTests)
{
	std::string promServerAddr = "localhost:8200";

	std::future<int> shResult;

	// For internal statistics
	PrometheusServer reporter(promServerAddr);

	// Init Telnet Server
	auto telnetServerPtr = std::make_shared<TelnetServer>();
	ASSERT_TRUE(telnetServerPtr->initialise(std::stoi(readSingleConfig(TEST_CONFIG_PATH, "TELNET_PORT")), "",
											reporter.createNewRegistry()));
	ASSERT_FALSE(telnetServerPtr->initialise(std::stoi(readSingleConfig(TEST_CONFIG_PATH, "TELNET_PORT"))));
	telnetServerPtr->shutdown();

	ASSERT_TRUE(telnetServerPtr->initialise(std::stoi(readSingleConfig(TEST_CONFIG_PATH, "TELNET_PORT")), "> "));
	telnetServerPtr->connectedCallback(TelnetConnectedCallback);
	telnetServerPtr->newLineCallback(TelnetMessageCallback);
	telnetServerPtr->tabCallback(TelnetTabCallback);

	// Launch script
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	shResult = std::async(std::launch::async, []() {
		return system(("expect " + std::string(TEST_TELNET_SH_PATH) + " >/dev/null").c_str());
	});

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	for (size_t idx = 0; idx < 250; ++idx)
	{
		telnetServerPtr->update();
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}

	shResult.wait();
	telnetServerPtr->shutdown();
	ASSERT_EQ(0, shResult.get());
}
