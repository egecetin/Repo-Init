#include "Control.hpp"
#include "Utils.hpp"
#include "telnet/TelnetServer.hpp"
#include "zeromq/ZeroMQServer.hpp"

#include <chrono>
#include <thread>

#include <spdlog/spdlog.h>

// GCOVR_EXCL_START
void telnetControlThread(const std::unique_ptr<PrometheusServer> &mainPrometheusServer, uint16_t telnetPort)
{
	// Init Telnet Server
	auto telnetServerPtr = std::make_shared<TelnetServer>();
	if (telnetPort > 0 &&
		telnetServerPtr->initialise(telnetPort, "> ",
									mainPrometheusServer ? mainPrometheusServer->createNewRegistry() : nullptr))
	{
		telnetServerPtr->connectedCallback(TelnetConnectedCallback);
		telnetServerPtr->newLineCallback(TelnetMessageCallback);
		telnetServerPtr->tabCallback(TelnetTabCallback);
		spdlog::info("Telnet server created at {}", telnetPort);
	}
	else
	{
		if (telnetPort > 0)
		{
			spdlog::warn("Can't start Telnet Server: {}", strerror(errno)); // NOLINT(concurrency-mt-unsafe)
		}
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
void zmqControlThread(const std::unique_ptr<PrometheusServer> &mainPrometheusServer, const std::string &serverAddr)
{
	// Init ZeroMQ server
	auto zeroMqServerPtr = std::make_shared<ZeroMQServer>();
	try
	{
		if (!serverAddr.empty() &&
			zeroMqServerPtr->initialise(serverAddr,
										mainPrometheusServer ? mainPrometheusServer->createNewRegistry() : nullptr))
		{
			zeroMqServerPtr->messageCallback(ZeroMQServerMessageCallback);
			spdlog::info("ZeroMQ server created at {}", serverAddr);
		}
		else
		{
			throw std::runtime_error("Unknown error");
		}
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
