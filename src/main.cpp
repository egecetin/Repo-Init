#include "Control.hpp"
#include "Utils.hpp"
#include "Version.h"
#include "logging/Logger.hpp"
#include "metrics/Reporter.hpp"

#include <csignal>
#include <thread>

#include <spdlog/spdlog.h>

int main(int argc, char **argv)
{
	// Parse inputs
	InputParser input(argc, argv);
	if (input.cmdOptionExists("--enable-telnet"))
	{
		std::string portString = input.getCmdOption("--enable-telnet");
		if (portString.size())
			TELNET_PORT = std::stoi(portString);
		else
			spdlog::warn("Enable Telnet option requires a port number");
	}
	if (input.cmdOptionExists("--enable-prometheus"))
	{
		PROMETHEUS_ADDR = input.getCmdOption("--enable-prometheus");
		if (PROMETHEUS_ADDR.empty())
			spdlog::warn("Enable Prometheus option requires a port number");
	}
	if (input.cmdOptionExists("--heartbeat-index"))
		HEARTBEAT_INDEX = input.getCmdOption("--heartbeat-index");
	if (input.cmdOptionExists("--config"))
		CONFIG_FILE_PATH = input.getCmdOption("--config");
	/* ################################################################################### */
	/* ############################# MAKE MODIFICATIONS HERE ############################# */
	/* ################################################################################### */

	/* ################################################################################### */
	/* ################################ END MODIFICATIONS ################################ */
	/* ################################################################################### */

	// Init logger
	MainLogger logger(argc, argv, CONFIG_FILE_PATH);
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [" + std::string(PROJECT_NAME) + "] [%^%l%$] : %v");
	print_version();

	// Read config
	if (!readConfig(CONFIG_FILE_PATH))
		return EXIT_FAILURE;

	// Register signals
	signal(SIGINT, interruptFunc);
	signal(SIGTERM, interruptFunc);
	signal(SIGKILL, interruptFunc);

	// Move SIGALRM to bottom because of invoking sleep

	// Init variables
	alarmCtr = 1; // Should be non-zero at startup
	currentTime = 0;
	loopFlag = true;

	ALARM_INTERVAL = 1;
	HEARTBEAT_INTERVAL = 20;
	/* ################################################################################### */
	/* ############################# MAKE MODIFICATIONS HERE ############################# */
	/* ################################################################################### */

	/* ################################################################################### */
	/* ################################ END MODIFICATIONS ################################ */
	/* ################################################################################### */

	// Init prometheus server
	if (PROMETHEUS_ADDR.size())
	{
		try
		{
			mainPrometheusHandler = new Reporter(PROMETHEUS_ADDR);
			spdlog::info("Prometheus server start at {}", PROMETHEUS_ADDR);
		}
		catch (const std::exception &e)
		{
			spdlog::warn("Can't start Prometheus Server: {}", e.what());
		}
	}

	// Start threads
	/* ################################################################################### */
	/* ############################# MAKE MODIFICATIONS HERE ############################# */
	/* ################################################################################### */

	/* ################################################################################### */
	/* ################################ END MODIFICATIONS ################################ */
	/* ################################################################################### */
	std::thread zmqControlTh(zmqControlThread);
	std::thread telnetControlTh(telnetControlThread);
	spdlog::debug("Threads started");

	// SIGALRM should be registered after all sleep calls
	signal(SIGALRM, alarmFunc);
	alarm(ALARM_INTERVAL);

	// Join threads
	if (zmqControlTh.joinable())
		zmqControlTh.join();
	spdlog::info("ZMQ Controller joined");
	if (telnetControlTh.joinable())
		telnetControlTh.join();
	spdlog::info("Telnet Controller joined");
	/* ################################################################################### */
	/* ############################# MAKE MODIFICATIONS HERE ############################# */
	/* ################################################################################### */

	/* ################################################################################### */
	/* ################################ END MODIFICATIONS ################################ */
	/* ################################################################################### */

	spdlog::info("{} Exited", PROJECT_NAME);
	return EXIT_SUCCESS;
}
