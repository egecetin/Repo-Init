#include "Control.hpp"
#include "Utils.hpp"
#include "Version.h"
#include "metrics/Reporter.hpp"

#include <signal.h>
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
	/* ############################# MAKE MODIFICATIONS HERE ############################# */

	/* ################################ END MODIFICATIONS ################################ */

	// Init logger
	if (!init_logger(argc, argv))
		return EXIT_FAILURE;

	// Read config
	if (!readConfig(CONFIG_FILE_PATH))
		return EXIT_FAILURE;

	// Register alarms
	signal(SIGINT, interruptFunc);
	signal(SIGTERM, interruptFunc);
	signal(SIGKILL, interruptFunc);

	// Register backtracer
	signal(SIGSEGV, backtracer);

	// Move SIGALRM to bottom because of invoking sleep

	// Init variables
	alarmCtr = 1; // Should be non-zero at startup
	currentTime = 0;
	loopFlag = true;
	sigReadyFlag = false;

	ALARM_INTERVAL = 1;
	HEARTBEAT_INTERVAL = 20;
	/* ############################# MAKE MODIFICATIONS HERE ############################# */

	/* ################################ END MODIFICATIONS ################################ */

	// Init prometheus server
	if (PROMETHEUS_ADDR.size())
		mainPrometheusHandler = new Reporter(PROMETHEUS_ADDR);

	// Start threads
	/* ############################# MAKE MODIFICATIONS HERE ############################# */

	/* ################################ END MODIFICATIONS ################################ */
	std::thread zmqControlTh(zmqControlThread);
	std::thread telnetControlTh(telnetControlThread);
	spdlog::debug("Threads started");

	// SIGALRM should be registered after all sleep calls
	signal(SIGALRM, alarmFunc);
	alarm(ALARM_INTERVAL);

	// Join threads
	if (telnetControlTh.joinable())
		telnetControlTh.join();
	spdlog::info("Telnet Controller joined");
	if (zmqControlTh.joinable())
		zmqControlTh.join();
	spdlog::info("ZMQ Controller joined");
	/* ############################# MAKE MODIFICATIONS HERE ############################# */

	/* ################################ END MODIFICATIONS ################################ */

	spdlog::warn("{} Exited", PROJECT_NAME);
	close_logger();

	return EXIT_SUCCESS;
}
