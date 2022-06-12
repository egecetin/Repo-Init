#include "Utils.h"
#include "TelnetServer.h"
#include "test-static-definitions.h"

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
	ASSERT_EQ("/tmp", CONTROL_IPC_PATH);

	ASSERT_EQ("1000", readSingleConfig(TEST_CONFIG_PATH, "ZMQ_SEND_TIMEOUT"));
	ASSERT_EQ("1000", readSingleConfig(TEST_CONFIG_PATH, "ZMQ_RECV_TIMEOUT"));
	ASSERT_EQ("/tmp", readSingleConfig(TEST_CONFIG_PATH, "CONTROL_IPC_PATH"));

	ASSERT_FALSE(readConfig("dummypath"));
	ASSERT_EQ("", readSingleConfig("dummypath", "ZMQ_SEND_TIMEOUT"));
	ASSERT_EQ("", readSingleConfig(TEST_CONFIG_PATH, "dummyoption"));
}

TEST(Utils_Tests, Telnet_Tests)
{
	ASSERT_FALSE(TelnetSession::UNIT_TEST());
}
