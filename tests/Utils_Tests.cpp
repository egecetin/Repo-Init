#include "Utils.hpp"
#include "test-static-definitions.h"

#include <gtest/gtest.h>

TEST(Utils_Tests, InputParserTests)
{
	// Test
	int nArgc = 4;
	char *nArgv[] = {"program", "argument1", "option1", "--argument2", NULL};
	InputParser parser(nArgc, nArgv);

	ASSERT_EQ("", parser.getCmdOption("program"));
	ASSERT_FALSE(parser.cmdOptionExists("program"));

	ASSERT_EQ("", parser.getCmdOption("unknownArg"));
	ASSERT_FALSE(parser.cmdOptionExists("unknownArg"));

	ASSERT_EQ("option1", parser.getCmdOption("argument1"));
	ASSERT_TRUE(parser.cmdOptionExists("argument1"));

	ASSERT_EQ("", parser.getCmdOption("--argument2"));
	ASSERT_TRUE(parser.cmdOptionExists("--argument2"));
}

TEST(Utils_Tests, ConfigReaderTests)
{
	ASSERT_TRUE(readConfig(TEST_CONFIG_PATH));

	ASSERT_FALSE(readConfig("dummypath"));
	ASSERT_EQ("", readSingleConfig("dummypath", "SENTRY_ADDRESS"));
	ASSERT_EQ("", readSingleConfig(TEST_CONFIG_PATH, "dummyoption"));

	ASSERT_FALSE(readConfig(TEST_CONFIG_EMPTY_PATH));
	ASSERT_EQ("", readSingleConfig(TEST_CONFIG_EMPTY_PATH, "SENTRY_ADDRESS"));
	ASSERT_EQ("", readSingleConfig(TEST_CONFIG_EMPTY_PATH, "dummyoption"));
}

TEST(Utils_Tests, OtherTests) { ASSERT_EQ(getErrnoString(EACCES), "Permission denied"); }
