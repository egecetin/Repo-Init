#include "utils/ConfigParser.hpp"
#include "utils/ErrorHelpers.hpp"
#include "utils/FileHelpers.hpp"
#include "utils/InputParser.hpp"

#include "test-static-definitions.h"

#include <gtest/gtest.h>

TEST(Utils_Tests, ConfigParserUnitTests)
{
	ConfigParser parser(TEST_CONFIG_PATH);

	ASSERT_EQ(parser.get("TELNET_PORT"), "23000");
	ASSERT_EQ(parser.get("LOKI_ADDRESS"), "http://localhost:8400");
	ASSERT_EQ(parser.get("NonExistentKey"), "");

	ASSERT_NO_THROW(parser.load());
	ASSERT_GT(parser.getConfigMap().size(), 0);

	ASSERT_NO_THROW(parser.set("NEW_KEY", "NEW_VALUE"));
	ASSERT_EQ(parser.get("NEW_KEY"), "NEW_VALUE");
	ASSERT_NO_THROW(parser.load());
	ASSERT_EQ(parser.get("NEW_KEY"), "");

	ASSERT_NO_THROW(parser.set("NEW_KEY", "NEW_VALUE"));
	ASSERT_NO_THROW(parser.save());

	ConfigParser parser2(TEST_CONFIG_PATH);
	ASSERT_EQ(parser2.get("NEW_KEY"), "NEW_VALUE");

	ASSERT_NO_THROW(parser.remove("NEW_KEY"));
	ASSERT_EQ(parser.get("NEW_KEY"), "");
	ASSERT_NO_THROW(parser.save());

	ASSERT_EQ(parser2.get("NEW_KEY"), "NEW_VALUE");
	ASSERT_NO_THROW(parser2.load());
	ASSERT_EQ(parser2.get("NEW_KEY"), "");

	ASSERT_THROW(ConfigParser(""), std::invalid_argument);
	ASSERT_THROW(ConfigParser(TEST_CONFIG_EMPTY_PATH), std::invalid_argument);
}

TEST(Utils_Tests, ErrorHelpersUnitTests)
{
	ASSERT_EQ(getErrnoString(0), "Success");
	ASSERT_EQ(getErrnoString(EACCES), "Permission denied");
}

TEST(Utils_Tests, FileHelpersUnitTests)
{
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

TEST(Utils_Tests, InputParserUnitTests)
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
