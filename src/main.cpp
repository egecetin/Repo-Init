#include "Control.hpp"
#include "Utils.hpp"
#include "Version.h"
#include "logging/Logger.hpp"
#include "metrics/PrometheusServer.hpp"

#include <csignal>
#include <thread>

#include <spdlog/spdlog.h>

int main(int argc, char **argv)
{
	int telnetPort = 0;
	std::string configPath = "config.json";

	// Parse inputs
	InputParser input(argc, argv);
	if (input.cmdOptionExists("--enable-telnet"))
	{
		std::string portString = input.getCmdOption("--enable-telnet");
		if (portString.size())
		{
			telnetPort = std::stoi(portString);
			if (telnetPort <= 0 || telnetPort > std::numeric_limits<uint16_t>::max())
			{
				spdlog::warn("Telnet port should be between [1-65535]: provided value is {}", telnetPort);
				telnetPort = 0;
			}
		}
		else
			spdlog::warn("Enable Telnet option requires a port number");
	}
	if (input.cmdOptionExists("--enable-prometheus"))
	{
		PROMETHEUS_ADDR = input.getCmdOption("--enable-prometheus");
		if (PROMETHEUS_ADDR.empty())
			spdlog::warn("Enable Prometheus option requires a bind address");
	}
	if (input.cmdOptionExists("--enable-zeromq"))
	{
		ZEROMQ_SERVER_PATH = input.getCmdOption("--enable-zeromq");
		if (ZEROMQ_SERVER_PATH.empty())
			spdlog::warn("Enable ZeroMQ option requires a connection address");
	}
	if (input.cmdOptionExists("--config"))
		configPath = input.getCmdOption("--config");
	/* ################################################################################### */
	/* ############################# MAKE MODIFICATIONS HERE ############################# */
	/* ################################################################################### */

	/* ################################################################################### */
	/* ################################ END MODIFICATIONS ################################ */
	/* ################################################################################### */

	// Init logger
	MainLogger logger(argc, argv, configPath);
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [" + std::string(PROJECT_NAME) + "] [%^%l%$] : %v");
	print_version();

	// Read config
	if (!readConfig(configPath))
		return EXIT_FAILURE;

	// Register signals
	signal(SIGINT, interruptFunc);
	signal(SIGTERM, interruptFunc);
	signal(SIGKILL, interruptFunc);

	// Move SIGALRM to bottom because of invoking sleep

	// Init variables
	alarmCtr = 0;
	loopFlag = true;

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
			mainPrometheusServer = new PrometheusServer(PROMETHEUS_ADDR);
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
	std::thread telnetControlTh(telnetControlThread, telnetPort);
	spdlog::debug("Threads started");

	// SIGALRM should be registered after all sleep calls
	signal(SIGALRM, alarmFunc);
	alarm(alarmInterval);

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
