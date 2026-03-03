#include "metrics/PrometheusServer.hpp"
#include "telnet/TelnetServer.hpp"
#include "TelnetClient.hpp"
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
	std::vector<std::string> commands = {"Test Message\r",     "Unknown Message\r", "help\r",
	                                     "\b",                 "\t",                "he\t\r",
	                                     "enable log v\r",     "enable log vv\r",   "ping\r",
	                                     "clear\r",            "enable log vvv\r",  "disable log\r",
	                                     "disable log all\r",  "version\r",         "status\r",
	                                     "\x1b\x5b\x41\r",     "\x1b\x5b\x42\r",    "\r",
	                                     "quit\r"};

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
