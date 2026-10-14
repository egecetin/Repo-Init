#include "TelnetClient.hpp"
#include "metrics/PrometheusServer.hpp"
#include "telnet/TelnetServer.hpp"
#include "test-static-definitions.h"

#include <chrono>
#include <memory>
#include <mutex>
#include <string>
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
										 "status",
										 "version",
										 "\x1b\x5b\x41",
										 "\x1b\x5b\x42",
										 "",
										 "disable log",
										 "disable log all",
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

	for (auto &client : additionalClients)
	{
		client->sendCommand("ping");
	}

	ASSERT_NO_THROW(telnetServerPtr->shutdown());
	ASSERT_NO_THROW(telnetServerPtr->shutdown());
}

TEST(Telnet_Tests, TelnetCoalescedCommandsUnitTests)
{
	constexpr int port = TELNET_PORT + 1;

	std::mutex mtx;
	std::vector<std::string> receivedLines;
	std::vector<std::string> tabRequests;

	auto telnetServerPtr = std::make_shared<TelnetServer>();
	std::shared_ptr<std::atomic_flag> checkFlag;
	ASSERT_TRUE(telnetServerPtr->initialise(port, checkFlag, "> "));
	telnetServerPtr->newLineCallback([&](const SP_TelnetSession &, const std::string &line) {
		const std::lock_guard<std::mutex> lock(mtx);
		receivedLines.push_back(line);
		return true;
	});
	telnetServerPtr->tabCallback([&](const SP_TelnetSession &, std::string_view line) {
		const std::lock_guard<std::mutex> lock(mtx);
		tabRequests.emplace_back(line);
		return line == "he" ? std::string("help") : std::string();
	});

	{
		TelnetClient client("127.0.0.1", port);

		// Sent in a single write so the server receives all of them in the same recv call
		client.sendCommand("\b\r\n\t\r\nhe\t\r\nping\r\n\x1b[A");
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
	}

	ASSERT_NO_THROW(telnetServerPtr->shutdown());

	const std::lock_guard<std::mutex> lock(mtx);
	EXPECT_EQ(receivedLines, (std::vector<std::string>{"", "", "help", "ping", "ping"}));
	EXPECT_EQ(tabRequests, (std::vector<std::string>{"", "he"}));
}
