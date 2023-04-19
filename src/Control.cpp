#include "Control.hpp"
#include "Utils.hpp"
#include "metrics/Reporter.hpp"
#include "telnet/TelnetServer.hpp"
#include "zeromq/ZeroMQServer.hpp"

#include <chrono>
#include <thread>

#include <spdlog/spdlog.h>

// GCOVR_EXCL_START
void telnetControlThread()
{
	// Init Telnet Server
	auto telnetServerPtr = std::make_shared<TelnetServer>();
	if (TELNET_PORT && telnetServerPtr->initialise(
						   TELNET_PORT, "> ", mainPrometheusHandler ? mainPrometheusHandler->getRegistry() : nullptr))
	{
		telnetServerPtr->connectedCallback(TelnetConnectedCallback);
		telnetServerPtr->newLineCallback(TelnetMessageCallback);
		telnetServerPtr->tabCallback(TelnetTabCallback);
		spdlog::info("Telnet server created at {}", TELNET_PORT);
	}
	else
	{
		if (TELNET_PORT)
			spdlog::warn("Can't start Telnet Server: {}", strerror(errno));
		return;
	}

	while (loopFlag)
	{
		try
		{
			telnetServerPtr->update();
		}
		catch (const std::exception &e)
		{
			spdlog::error("Telnet failed: {}", e.what());
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	// Closing server
	telnetServerPtr->shutdown();
	spdlog::debug("Telnet Control thread done");
}
// GCOVR_EXCL_STOP

// GCOVR_EXCL_START
void zmqControlThread()
{
	// Init ZeroMQ server
	auto zeroMqServerPtr = std::make_shared<ZeroMQServer>();
	try
	{
		if (!ZEROMQ_SERVER_PATH.empty() &&
			zeroMqServerPtr->initialise(ZEROMQ_SERVER_PATH,
										mainPrometheusHandler ? mainPrometheusHandler->getRegistry() : nullptr))
		{
			zeroMqServerPtr->messageCallback(ZeroMQServerMessageCallback);
			spdlog::info("ZeroMQ server created at {}", ZEROMQ_SERVER_PATH);
		}
		else if (!ZEROMQ_SERVER_PATH.empty())
			throw std::runtime_error("Unknown error");
	}
	catch (const std::exception &e)
	{
		spdlog::warn("Can't start ZeroMQ server: {}", e.what());
		return;
	}

	while (loopFlag)
	{
		try
		{
			zeroMqServerPtr->update();
		}
		catch (const std::exception &e)
		{
			spdlog::error("ZeroMQ server failed: {}", e.what());
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	// Closing server
	zeroMqServerPtr->shutdown();
	spdlog::debug("ZeroMQ Control thread done");
}
// GCOVR_EXCL_STOP
