#include "test-static-definitions.h"

#include "logging/Logger.hpp"

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

TEST(Logging_Tests, PlainLogger)
{
	MainLogger logger(0, nullptr, TEST_CONFIG_PATH);

	logger.getLogger()->trace("Trace Message");
	logger.getLogger()->debug("Debug Message");
	logger.getLogger()->info("Info Message");
	logger.getLogger()->warn("Warn Message");
	logger.getLogger()->error("Error Message");
	logger.getLogger()->critical("Critical Message");
}

TEST(Logging_Tests, ModifiedLoggerWithConfig)
{
	int nArgc = 2;
	char *nArgv[] = {"program", "-v", NULL};
	MainLogger logger(nArgc, nArgv, TEST_CONFIG_PATH);

	logger.getLogger()->trace("Trace Message");
	logger.getLogger()->debug("Debug Message");
	logger.getLogger()->info("Info Message");
	logger.getLogger()->warn("Warn Message");
	logger.getLogger()->error("Error Message");
	logger.getLogger()->critical("Critical Message");
	logger.getLogger()->critical("Critical Message 2");

	nArgv[1] = "-vv";
	logger = MainLogger(nArgc, nArgv, TEST_CONFIG_PATH);

	logger.getLogger()->trace("Trace Message");
	logger.getLogger()->debug("Debug Message");
	logger.getLogger()->info("Info Message");
	logger.getLogger()->warn("Warn Message");
	logger.getLogger()->error("Error Message");
	logger.getLogger()->critical("Critical Message");
	logger.getLogger()->critical("Critical Message 2");

	nArgv[1] = "-vvv";
	logger = MainLogger(nArgc, nArgv, TEST_CONFIG_PATH);

	logger.getLogger()->trace("Trace Message");
	logger.getLogger()->debug("Debug Message");
	logger.getLogger()->info("Info Message");
	logger.getLogger()->warn("Warn Message");
	logger.getLogger()->error("Error Message");
	logger.getLogger()->critical("Critical Message");
	logger.getLogger()->critical("Critical Message 2");
}

TEST(Logging_Tests, ModifiedLoggerWithoutConfig)
{
	int nArgc = 2;
	char *nArgv[] = {"program", "-v", NULL};
	MainLogger logger(nArgc, nArgv, TEST_CONFIG_EMPTY_PATH);

	logger.getLogger()->trace("Trace Message");
	logger.getLogger()->debug("Debug Message");
	logger.getLogger()->info("Info Message");
	logger.getLogger()->warn("Warn Message");
	logger.getLogger()->error("Error Message");
	logger.getLogger()->critical("Critical Message");
	logger.getLogger()->critical("Critical Message 2");

	nArgv[1] = "-vv";
	logger = MainLogger(nArgc, nArgv, TEST_CONFIG_EMPTY_PATH);

	logger.getLogger()->trace("Trace Message");
	logger.getLogger()->debug("Debug Message");
	logger.getLogger()->info("Info Message");
	logger.getLogger()->warn("Warn Message");
	logger.getLogger()->error("Error Message");
	logger.getLogger()->critical("Critical Message");
	logger.getLogger()->critical("Critical Message 2");

	nArgv[1] = "-vvv";
	logger = MainLogger(nArgc, nArgv, TEST_CONFIG_EMPTY_PATH);

	logger.getLogger()->trace("Trace Message");
	logger.getLogger()->debug("Debug Message");
	logger.getLogger()->info("Info Message");
	logger.getLogger()->warn("Warn Message");
	logger.getLogger()->error("Error Message");
	logger.getLogger()->critical("Critical Message");
	logger.getLogger()->critical("Critical Message 2");
}
