#include "Control.hpp"
#include "Tracer.hpp"
#include "Utils.hpp"
#include "Version.h"
#include "logging/Logger.hpp"
#include "metrics/PrometheusServer.hpp"

#include <csignal>
#include <thread>

#include <curl/curl.h>
#include <spdlog/spdlog.h>

int main(int argc, char **argv)
{
	std::string configPath = "config.json";

	int telnetPort = 0;
	std::string zeromqServerAddr;

	std::string prometheusAddr;
	std::unique_ptr<PrometheusServer> mainPrometheusServer;

	std::string dumpHandlerExecutable;
	std::string dumpServer, dumpServerProxy;
	std::unique_ptr<Tracer> crashdump;

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
	if (input.cmdOptionExists("--crashpad-exe"))
	{
		dumpHandlerExecutable = input.getCmdOption("--crahpad-exe");
	}
	if (input.cmdOptionExists("--crashpad-remote"))
	{
		dumpServer = input.getCmdOption("--crashpad-remote");
	}
	if (input.cmdOptionExists("--crashpad-remote-proxy"))
	{
		dumpServerProxy = input.getCmdOption("--crashpad-remote-proxy");
	}
	if (input.cmdOptionExists("--config"))
	{
		configPath = input.getCmdOption("--config");
	}

	if (curl_global_init(CURL_GLOBAL_DEFAULT) < 0)
	{
		spdlog::critical("Can't init curl");
		return EXIT_FAILURE;
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
		spdlog::critical("Can't set signal handler (SIGINT): {}", getErrnoString(errno));
		return EXIT_FAILURE;
	}
	if (signal(SIGTERM, interruptFunc) == SIG_ERR)
	{
		spdlog::critical("Can't set signal handler (SIGTERM): {}", getErrnoString(errno));
		return EXIT_FAILURE;
	}

	// Move SIGALRM to bottom because of invoking sleep

	// Init variables
	loopFlag = true;

	vCheckFlag.emplace_back("ZeroMQ Server", std::make_unique<std::atomic_flag>(false));
	vCheckFlag.emplace_back("Telnet Server", std::make_unique<std::atomic_flag>(false));

	// Start crashpad handler
	try
	{
		crashdump =
			std::make_unique<Tracer>(dumpServer, dumpServerProxy, dumpHandlerExecutable,
									 std::map<std::string, std::string>(
										 {{"name", PROJECT_NAME},
										  {"version", PROJECT_FULL_REVISION},
										  {"build_info", PROJECT_BUILD_DATE + std::string(" ") + PROJECT_BUILD_TIME +
															 std::string(" ") + BUILD_TYPE},
										  {"compiler_info", COMPILER_NAME + std::string(" ") + COMPILER_VERSION}}),
									 std::vector<base::FilePath>());
	}
	catch(const std::exception& e)
	{
		spdlog::error("Can't start crashpad: {}", e.what());
		return EXIT_FAILURE;
	}

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
	std::unique_ptr<std::thread> zmqControlTh(nullptr);
	std::unique_ptr<std::thread> telnetControlTh(nullptr);

	if (!zeromqServerAddr.empty())
	{
		zmqControlTh = std::make_unique<std::thread>(zmqControlThread, std::ref(mainPrometheusServer),
													 std::ref(zeromqServerAddr), std::ref(vCheckFlag[0].second));
	}
	if (telnetPort > 0)
	{
		telnetControlTh = std::make_unique<std::thread>(telnetControlThread, std::ref(mainPrometheusServer), telnetPort,
														std::ref(vCheckFlag[1].second));
	}
	spdlog::debug("Threads started");

	// SIGALRM should be registered after all sleep calls
	if (signal(SIGALRM, alarmFunc) == SIG_ERR)
	{
		spdlog::critical("Can't set signal handler (SIGALRM): {}", getErrnoString(errno));
		return EXIT_FAILURE;
	}
	alarm(alarmInterval);

	// Join threads
	if (zmqControlTh && zmqControlTh->joinable())
	{
		zmqControlTh->join();
		spdlog::info("ZMQ Controller joined");
	}
	if (telnetControlTh && telnetControlTh->joinable())
	{
		telnetControlTh->join();
		spdlog::info("Telnet Controller joined");
	}
	/* ################################################################################### */
	/* ############################# MAKE MODIFICATIONS HERE ############################# */
	/* ################################################################################### */

	/* ################################################################################### */
	/* ################################ END MODIFICATIONS ################################ */
	/* ################################################################################### */

	curl_global_cleanup();

	spdlog::info("{} Exited", PROJECT_NAME);
	return EXIT_SUCCESS;
}
