#include "Control.hpp"
#include "Utils.hpp"
#include "Version.h"
#include "logging/Logger.hpp"
#include "metrics/ProcessMetrics.hpp"
#include "metrics/PrometheusServer.hpp"
#include "telnet/TelnetServer.hpp"
#include "zeromq/ZeroMQServer.hpp"

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
	if (!readAndVerifyConfig(configPath))
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
	vCheckFlag.emplace_back("Crashpad Handler", std::make_unique<std::atomic_flag>(false));
	vCheckFlag.emplace_back("Self Monitor", std::make_unique<std::atomic_flag>(false));

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
			loopFlag = false;
		}
	}

	// Start ZeroMQ server if adress is provided
	std::unique_ptr<ZeroMQServer> zmqController(nullptr);
	if (!zeromqServerAddr.empty())
	{
		try
		{
			zmqController = std::make_unique<ZeroMQServer>(
				zeromqServerAddr, vCheckFlag[0].second,
				mainPrometheusServer ? mainPrometheusServer->createNewRegistry() : nullptr);
			zmqController->messageCallback(ZeroMQServerMessageCallback);
			zmqController->initialise();
		}
		catch (const std::exception &e)
		{
			spdlog::warn("Can't start ZeroMQ Server: {}", e.what());
			loopFlag = false;
		}
	}

	// Start Telnet server if port is provided
	std::unique_ptr<TelnetServer> telnetController(nullptr);
	if (telnetPort > 0)
	{
		try
		{
			telnetController = std::make_unique<TelnetServer>();
			telnetController->connectedCallback(TelnetConnectedCallback);
			telnetController->newLineCallback(TelnetMessageCallback);
			telnetController->tabCallback(TelnetTabCallback);
			telnetController->initialise(telnetPort, "> ",
										 mainPrometheusServer ? mainPrometheusServer->createNewRegistry() : nullptr);
		}
		catch (const std::exception &e)
		{
			spdlog::warn("Can't start Telnet Server: {}", e.what());
			loopFlag = false;
		}
	}

	// Start Crashpad handler
	std::unique_ptr<Tracer> crashpadController(nullptr);
	crashpadController = std::make_unique<Tracer>(
		readSingleConfig(configPath, "CRASHPAD_REMOTE"), readSingleConfig(configPath, "CRASHPAD_PROXY"),
		readSingleConfig(configPath, "CRASHPAD_EXECUTABLE_DIR"),
		std::map<std::string, std::string>(
			{{"name", PROJECT_NAME},
			 {"version", PROJECT_FULL_REVISION},
			 {"build_info", PROJECT_BUILD_DATE + std::string(" ") + PROJECT_BUILD_TIME + std::string(" ") + BUILD_TYPE},
			 {"compiler_info", COMPILER_NAME + std::string(" ") + COMPILER_VERSION}}),
		std::vector<base::FilePath>({base::FilePath(configPath)}), readSingleConfig(configPath, "CRASHPAD_REPORT_DIR"),
		vCheckFlag[2].second, mainPrometheusServer ? mainPrometheusServer->createNewRegistry() : nullptr);

	// Start self monitor
	std::unique_ptr<ProcessMetrics> selfMonitor(nullptr);
	selfMonitor = std::make_unique<ProcessMetrics>(
		vCheckFlag[3].second, mainPrometheusServer ? mainPrometheusServer->createNewRegistry() : nullptr);
	/* ################################################################################### */
	/* ############################# MAKE MODIFICATIONS HERE ############################# */
	/* ################################################################################### */

	/* ################################################################################### */
	/* ################################ END MODIFICATIONS ################################ */
	/* ################################################################################### */
	spdlog::debug("Threads started");

	// SIGALRM should be registered after all sleep calls
	if (signal(SIGALRM, alarmFunc) == SIG_ERR)
	{
		spdlog::critical("Can't set signal handler (SIGALRM): {}", getErrnoString(errno));
		return EXIT_FAILURE;
	}
	alarm(alarmInterval);

	/* ################################################################################### */
	/* ############################# MAKE MODIFICATIONS HERE ############################# */
	/* ################################################################################### */

	/* ################################################################################### */
	/* ################################ END MODIFICATIONS ################################ */
	/* ################################################################################### */
	spdlog::debug("Threads stopped");

	curl_global_cleanup();

	spdlog::info("{} Exited", PROJECT_NAME);
	return EXIT_SUCCESS;
}
