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
	std::string zeromqServerAddr;

	std::string prometheusAddr;
	std::unique_ptr<PrometheusServer> mainPrometheusServer;

	// Parse inputs
	const InputParser input(argc, argv);
	if (input.cmdOptionExists("--enable-telnet"))
	{
		const std::string portString = input.getCmdOption("--enable-telnet");
		if (!portString.empty())
		{
			telnetPort = std::stoi(portString);
			if (telnetPort <= 0 || telnetPort > std::numeric_limits<uint16_t>::max())
			{
				spdlog::warn("Telnet port should be between [1-65535]: provided value is {}", telnetPort);
				telnetPort = 0;
			}
		}
		else
		{
			spdlog::warn("Enable Telnet option requires a port number");
		}
	}
	if (input.cmdOptionExists("--enable-prometheus"))
	{
		prometheusAddr = input.getCmdOption("--enable-prometheus");
		if (prometheusAddr.empty())
		{
			spdlog::warn("Enable Prometheus option requires a bind address");
		}
	}
	if (input.cmdOptionExists("--enable-zeromq"))
	{
		zeromqServerAddr = input.getCmdOption("--enable-zeromq");
		if (zeromqServerAddr.empty())
		{
			spdlog::warn("Enable ZeroMQ option requires a connection address");
		}
	}
	if (input.cmdOptionExists("--config"))
	{
		configPath = input.getCmdOption("--config");
	}
	/* ################################################################################### */
	/* ############################# MAKE MODIFICATIONS HERE ############################# */
	/* ################################################################################### */

	/* ################################################################################### */
	/* ################################ END MODIFICATIONS ################################ */
	/* ################################################################################### */

	// Init logger
	const MainLogger logger(argc, argv, configPath);
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [" + std::string(PROJECT_NAME) + "] [%^%l%$] : %v");
	print_version();

	// Read config
	if (!readConfig(configPath))
	{
		return EXIT_FAILURE;
	}

	// Register signals
	if (signal(SIGINT, interruptFunc) == SIG_ERR)
	{
		spdlog::critical("Can't set signal handler (SIGINT): {}", strerror(errno)); // NOLINT(concurrency-mt-unsafe)
		return EXIT_FAILURE;
	}
	if (signal(SIGTERM, interruptFunc) == SIG_ERR)
	{
		spdlog::critical("Can't set signal handler (SIGTERM): {}", strerror(errno)); // NOLINT(concurrency-mt-unsafe)
		return EXIT_FAILURE;
	}
	if (signal(SIGKILL, interruptFunc) == SIG_ERR)
	{
		spdlog::critical("Can't set signal handler (SIGKILL): {}", strerror(errno)); // NOLINT(concurrency-mt-unsafe)
		return EXIT_FAILURE;
	}

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
	if (!prometheusAddr.empty())
	{
		try
		{
			mainPrometheusServer = std::make_unique<PrometheusServer>(prometheusAddr);
			spdlog::info("Prometheus server start at {}", prometheusAddr);
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
	std::thread zmqControlTh(zmqControlThread, std::ref(mainPrometheusServer), std::ref(zeromqServerAddr));
	std::thread telnetControlTh(telnetControlThread, std::ref(mainPrometheusServer), telnetPort);
	spdlog::debug("Threads started");

	// SIGALRM should be registered after all sleep calls
	if (signal(SIGALRM, alarmFunc) == SIG_ERR)
	{
		spdlog::critical("Can't set signal handler (SIGALRM): {}", strerror(errno)); // NOLINT(concurrency-mt-unsafe)
		return EXIT_FAILURE;
	}
	alarm(alarmInterval);

	// Join threads
	if (zmqControlTh.joinable())
	{
		zmqControlTh.join();
		spdlog::info("ZMQ Controller joined");
	}
	if (telnetControlTh.joinable())
	{
		telnetControlTh.join();
		spdlog::info("Telnet Controller joined");
	}
	/* ################################################################################### */
	/* ############################# MAKE MODIFICATIONS HERE ############################# */
	/* ################################################################################### */

	/* ################################################################################### */
	/* ################################ END MODIFICATIONS ################################ */
	/* ################################################################################### */

	spdlog::info("{} Exited", PROJECT_NAME);
	return EXIT_SUCCESS;
}
