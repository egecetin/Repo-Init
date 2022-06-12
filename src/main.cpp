#include "Control.h"
#include "Utils.h"

#include <signal.h>
#include <thread>

#include <spdlog/spdlog.h>

int main(int argc, char **argv)
{
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [XXX] [%^%l%$] : %v");
	print_version();

#ifdef NDEBUG
	spdlog::set_level(spdlog::level::warn);
#else
	spdlog::set_level(spdlog::level::info);
#endif

	// Parse input arguments
	InputParser input(argc, argv);
	if (input.cmdOptionExists("-v"))
		spdlog::set_level(spdlog::level::info);
	if (input.cmdOptionExists("-vv"))
		spdlog::set_level(spdlog::level::debug);
	if (input.cmdOptionExists("-vvv"))
		spdlog::set_level(spdlog::level::trace);

	// Read config
	if (!readConfig(CONFIG_FILE_PATH))
		return -1;

	// Register alarms
	signal(SIGINT, interruptFunc);
	signal(SIGTERM, interruptFunc);
	signal(SIGKILL, interruptFunc);

	signal(SIGSEGV, backtracer);
	// Move SIGALRM to bottom because of invoking sleep

	// Init variables
	alarmCtr = 1;
	currentTime = 0;
	loopFlag = true;
	sigReadyFlag = false;

	ALARM_INTERVAL = 1;

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
	if (zmqControlTh.joinable())
		zmqControlTh.join();
	spdlog::warn("Controller joined");

	spdlog::warn("Decryptor Exit");
	return EXIT_SUCCESS;
}
