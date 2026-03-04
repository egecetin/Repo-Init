#include "TelnetClient.hpp"
#include "metrics/PrometheusServer.hpp"
#include "telnet/TelnetServer.hpp"
#include "test-static-definitions.h"

#include <chrono>
#include <memory>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

constexpr int TELNET_PORT = 23000;

TEST(Telnet_Tests, TelnetServerUnitTests)
{
	std::string promServerAddr = "localhost:8200";

	// For internal statistics
	PrometheusServer reporter(promServerAddr);

	// Init Telnet Server
	auto telnetServerPtr = std::make_shared<TelnetServer>();
	std::shared_ptr<std::atomic_flag> checkFlag;
	ASSERT_TRUE(telnetServerPtr->initialise(TELNET_PORT, checkFlag, "", reporter.createNewRegistry()));
	ASSERT_FALSE(telnetServerPtr->initialise(TELNET_PORT, checkFlag));
	ASSERT_NO_THROW(telnetServerPtr->shutdown());

	ASSERT_TRUE(telnetServerPtr->initialise(TELNET_PORT, checkFlag, "> "));
	telnetServerPtr->connectedCallback(TelnetConnectedCallback);
	telnetServerPtr->newLineCallback(TelnetMessageCallback);
	telnetServerPtr->tabCallback(TelnetTabCallback);

	// Prepare commands to send
	std::vector<std::string> commands = {"Test Message",
										 "Unknown Message",
										 "help",
										 "\b",
										 "\t",
										 "he\t",
										 "enable log v",
										 "enable log vv",
										 "ping",
										 "clear",
										 "enable log vvv",
										 "disable log",
										 "disable log all",
										 "version",
										 "status",
										 "\x1b\x5b\x41",
										 "\x1b\x5b\x42",
										 "",
										 "quit"};

	// Create first client that sends all commands
	auto mainClient = std::make_unique<TelnetClient>("127.0.0.1", TELNET_PORT, commands);
	mainClient->wait();

	// Create additional connections
	std::vector<std::unique_ptr<TelnetClient>> additionalClients;
	for (int i = 0; i < 8; ++i)
	{
		additionalClients.push_back(std::make_unique<TelnetClient>("127.0.0.1", TELNET_PORT));
	}

	ASSERT_NO_THROW(telnetServerPtr->shutdown());
	ASSERT_NO_THROW(telnetServerPtr->shutdown());
}
