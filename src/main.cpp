#include "Control.h"
#include "Utils.h"

#include <signal.h>
#include <thread>

#include <spdlog/spdlog.h>

int main(int argc, char **argv)
{
	// Init logger
	if (!init_logger(argc, argv))
		return -1;

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
	alarmCtr = 1; // Should be non-zero at startup
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
	spdlog::warn("Telnet Controller joined");
	if (zmqControlTh.joinable())
		zmqControlTh.join();
	spdlog::warn("ZMQ Controller joined");

	spdlog::warn("Decryptor Exit");
	return EXIT_SUCCESS;
}
