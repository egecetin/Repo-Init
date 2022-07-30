#include "Control.h"
#include "Utils.h"

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

	// If sentry is not available register plain backtracer
	if (readSingleConfig(CONFIG_FILE_PATH, "SENTRY_ADDRESS").empty())
		signal(SIGSEGV, backtracer);

	// Move SIGALRM to bottom because of invoking sleep

	// Init variables
	alarmCtr = 1; // Should be non-zero at startup
	currentTime = 0;
	loopFlag = true;
	sigReadyFlag = false;

	ALARM_INTERVAL = 1;
	HEARTBEAT_INTERVAL = 20;

	// Start threads
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

	spdlog::warn("XXX Exited");
	close_logger();

	return EXIT_SUCCESS;
}
