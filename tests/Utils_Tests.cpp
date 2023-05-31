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

TEST(Utils_Tests, OtherTests)
{
	std::array<char, 33> envVar{"TEST_ENV_VAR=testEnvVarValue"};
	ASSERT_FALSE(putenv(envVar.data()));
	ASSERT_EQ(getEnvVar("TEST_ENV_VAR"), "testEnvVarValue");

	ASSERT_EQ(getErrnoString(EACCES), "Permission denied");

	auto readLines = findFromFile(TEST_DATA_READ_PATH, "^(cpu family)");
	ASSERT_EQ(readLines.size(), 2);
	ASSERT_EQ(readLines[0], "cpu family      : 6");
	ASSERT_EQ(readLines[1], "cpu family      : 6");

	std::string word;
	readLines = findFromFile(TEST_DATA_READ_PATH, "^processor", word);
	ASSERT_EQ(readLines.size(), 2);
	ASSERT_EQ(word, "6");
	ASSERT_EQ(readLines[0], "processor       : 6");
	ASSERT_EQ(readLines[1], "processor       : 7");

	readLines = findFromFile(TEST_DATA_READ_PATH, "^dummy", word);
	ASSERT_TRUE(readLines.empty());
}
