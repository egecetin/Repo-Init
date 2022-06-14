#include "Control.h"
#include "test-static-definitions.h"

#include <chrono>
#include <future>
#include <thread>

#include <gtest/gtest.h>

TEST(Utils_Tests, InputParser_Tests)
{
	// Test
	int nArgc = 4;
	char *nArgv[] = {"program", "argument1", "option1", "--argument2", NULL};
	InputParser parser(nArgc, nArgv);

	ASSERT_EQ("", parser.getCmdOption("program"));
	ASSERT_FALSE(parser.cmdOptionExists("program"));

	ASSERT_EQ("option1", parser.getCmdOption("argument1"));
	ASSERT_TRUE(parser.cmdOptionExists("argument1"));

	ASSERT_EQ("", parser.getCmdOption("--argument2"));
	ASSERT_TRUE(parser.cmdOptionExists("--argument2"));

	ASSERT_EQ("", parser.getCmdOption("unknownArg"));
	ASSERT_FALSE(parser.cmdOptionExists("unknownArg"));
}

TEST(Utils_Tests, ConfigReader_Tests)
{
	ASSERT_TRUE(readConfig(TEST_CONFIG_PATH));

	ASSERT_EQ(1000, ZMQ_SEND_TIMEOUT);
	ASSERT_EQ(1000, ZMQ_RECV_TIMEOUT);
	ASSERT_EQ("ipc:///tmp", CONTROL_IPC_PATH);

	ASSERT_EQ("1000", readSingleConfig(TEST_CONFIG_PATH, "ZMQ_SEND_TIMEOUT"));
	ASSERT_EQ("1000", readSingleConfig(TEST_CONFIG_PATH, "ZMQ_RECV_TIMEOUT"));
	ASSERT_EQ("ipc:///tmp", readSingleConfig(TEST_CONFIG_PATH, "CONTROL_IPC_PATH"));

	ASSERT_FALSE(readConfig("dummypath"));
	ASSERT_EQ("", readSingleConfig("dummypath", "ZMQ_SEND_TIMEOUT"));
	ASSERT_EQ("", readSingleConfig(TEST_CONFIG_PATH, "dummyoption"));
}

TEST(Utils_Tests, Telnet_Tests)
{
	// Internally used by Telnet Server sessions
	struct timespec ts;
	clock_gettime(CLOCK_TAI, &ts);
	currentTime = ts.tv_sec;

	// Internal tests
	ASSERT_FALSE(TelnetSession::UNIT_TEST());

	std::future<int> shResult;

	// Init Telnet Server
	auto telnetServerPtr = std::make_shared<TelnetServer>();
	ASSERT_TRUE(telnetServerPtr->initialise(std::stoi(readSingleConfig(TEST_CONFIG_PATH, "TELNET_PORT"))));
	telnetServerPtr->shutdown();

	ASSERT_TRUE(telnetServerPtr->initialise(std::stoi(readSingleConfig(TEST_CONFIG_PATH, "TELNET_PORT")), "> "));
	telnetServerPtr->connectedCallback(TelnetConnectedCallback);
	telnetServerPtr->newLineCallback(TelnetMessageCallback);

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	shResult =
		std::async(std::launch::async, []() { return system(("expect " + std::string(TEST_TELNET_SH_PATH)).c_str()); });

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	for (size_t idx = 0; idx < 100; ++idx)
	{
		telnetServerPtr->update();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	shResult.wait();
	telnetServerPtr->shutdown();

	ASSERT_EQ(0, shResult.get());
}
