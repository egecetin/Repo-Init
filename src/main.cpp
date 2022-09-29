#include "Control.hpp"
#include "Utils.hpp"
#include "Version.h"
#include "metrics/Reporter.hpp"
#include "rng/LehmerRNG.hpp"

#include <signal.h>
#include <thread>

#include <obfuscate.h>
#include <spdlog/spdlog.h>

int main(int argc, char **argv)
{
	// Parse inputs
	InputParser input(argc, argv);
	if (input.cmdOptionExists(static_cast<char *>(AY_OBFUSCATE_KEY("--enable-telnet", mlcg::seed(__FILE__, __LINE__)))))
	{
		std::string portString = input.getCmdOption(
			static_cast<char *>(AY_OBFUSCATE_KEY("--enable-telnet", mlcg::seed(__FILE__, __LINE__))));
		if (portString.size())
			TELNET_PORT = std::stoi(portString);
		else
			spdlog::warn(static_cast<char *>(
				AY_OBFUSCATE_KEY("Enable Telnet option requires a port number", mlcg::seed(__FILE__, __LINE__))));
	}
	if (input.cmdOptionExists(
			static_cast<char *>(AY_OBFUSCATE_KEY("--enable-prometheus", mlcg::seed(__FILE__, __LINE__)))))
	{
		PROMETHEUS_ADDR = input.getCmdOption(
			static_cast<char *>(AY_OBFUSCATE_KEY("--enable-prometheus", mlcg::seed(__FILE__, __LINE__))));
		if (PROMETHEUS_ADDR.empty())
			spdlog::warn(static_cast<char *>(
				AY_OBFUSCATE_KEY("Enable Prometheus option requires a port number", mlcg::seed(__FILE__, __LINE__))));
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
	spdlog::debug(static_cast<char *>(AY_OBFUSCATE_KEY("Threads started", mlcg::seed(__FILE__, __LINE__))));

	// SIGALRM should be registered after all sleep calls
	signal(SIGALRM, alarmFunc);
	alarm(ALARM_INTERVAL);

	// Join threads
	if (telnetControlTh.joinable())
		telnetControlTh.join();
	spdlog::info(static_cast<char *>(AY_OBFUSCATE_KEY("Telnet Controller joined", mlcg::seed(__FILE__, __LINE__))));
	if (zmqControlTh.joinable())
		zmqControlTh.join();
	spdlog::info(static_cast<char *>(AY_OBFUSCATE_KEY("ZMQ Controller joined", mlcg::seed(__FILE__, __LINE__))));
	/* ############################# MAKE MODIFICATIONS HERE ############################# */

	/* ################################ END MODIFICATIONS ################################ */

	spdlog::warn(static_cast<char *>(AY_OBFUSCATE_KEY("{} Exited", mlcg::seed(__FILE__, __LINE__))), PROJECT_NAME);
	close_logger();

	return EXIT_SUCCESS;
}
