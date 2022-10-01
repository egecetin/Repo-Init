#include "Control.hpp"
#include "Version.h"
#include "metrics/Performance.hpp"
#include "metrics/Reporter.hpp"
#include "metrics/Status.hpp"
#include "rng/Hasher.hpp"

#include <chrono>
#include <thread>

#include <curl/curl.h>
#include <spdlog/spdlog.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

size_t writeData(void *, size_t size, size_t nmemb, void *) { return size * nmemb; }

// GCOVR_EXCL_START
void telnetControlThread()
{
	// Init Telnet Server
	auto telnetServerPtr = std::make_shared<TelnetServer>();

	// Init performance tracker if prometheus enabled
	std::shared_ptr<PerformanceTracker> telnetPerformanceTracker;
	if (mainPrometheusHandler)
		telnetPerformanceTracker = mainPrometheusHandler->addNewPerfTracker("telnet_server");
	else
		telnetPerformanceTracker = nullptr;

	std::shared_ptr<StatusTracker> telnetStatusTracker;
	if (mainPrometheusHandler)
		telnetStatusTracker = mainPrometheusHandler->addNewStatTracker("telnet_server");
	else
		telnetStatusTracker = nullptr;

start:
	if (TELNET_PORT && telnetServerPtr->initialise(TELNET_PORT, "> ", telnetStatusTracker))
	{
		telnetServerPtr->connectedCallback(TelnetConnectedCallback);
		telnetServerPtr->newLineCallback(TelnetMessageCallback);
		telnetServerPtr->tabCallback(TelnetTabCallback);
		spdlog::info("Telnet server created at {}", TELNET_PORT);
	}
	else
	{
		for (size_t idx = 0; idx < 15; ++idx)
		{
			std::this_thread::sleep_for(std::chrono::seconds(2));
			if (!loopFlag)
				return;
		}
		goto start;
	}

	while (loopFlag)
	{
		try
		{
			// Update Telnet connection
			if (telnetPerformanceTracker)
				telnetPerformanceTracker->startTimer();
			telnetServerPtr->update();
			if (telnetPerformanceTracker)
				telnetPerformanceTracker->endTimer();
		}
		catch (const std::exception &e)
		{
			spdlog::error("Telnet {}", e.what());
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
	// Init ZMQ connection
	zmq::context_t ctx(1);
	zmq::socket_t socketRep(ctx, zmq::socket_type::rep);
	socketRep.set(zmq::sockopt::linger, 0);
	socketRep.set(zmq::sockopt::sndtimeo, ZMQ_SEND_TIMEOUT);
	socketRep.set(zmq::sockopt::rcvtimeo, ZMQ_RECV_TIMEOUT);
	socketRep.set(zmq::sockopt::heartbeat_ivl, 1000);
	socketRep.set(zmq::sockopt::heartbeat_ttl, 3000);
	socketRep.set(zmq::sockopt::heartbeat_timeout, 3000);

	std::string hostAddrRep = CONTROL_IPC_PATH + "/" + PROJECT_NAME;
	try
	{
		socketRep.bind(hostAddrRep);
		spdlog::debug("Receiver created to {}", hostAddrRep);
	}
	catch (const std::exception &e)
	{
		spdlog::critical("Can't bind to {} {}", hostAddrRep, e.what());
		loopFlag = false;
	}

	// Prepare heartbeat
	CURL *curl = nullptr;
	size_t oldCtr = alarmCtr;
	std::string HEARTBEAT_ADDRESS = readSingleConfig(CONFIG_FILE_PATH, "HEARTBEAT_ADDRESS");
	if (!HEARTBEAT_ADDRESS.empty())
	{
		curl = curl_easy_init();
		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_URL, HEARTBEAT_ADDRESS.c_str());
			curl_easy_setopt(curl, CURLOPT_POST, 1L);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 100);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
		}
	}

	// Init performance and status tracker if prometheus enabled
	std::shared_ptr<PerformanceTracker> zmqControlPerformanceTracker;
	if (mainPrometheusHandler)
		zmqControlPerformanceTracker = mainPrometheusHandler->addNewPerfTracker("zmq_control_server");
	else
		zmqControlPerformanceTracker = nullptr;

	std::shared_ptr<StatusTracker> zmqControlStatusTracker;
	if (mainPrometheusHandler)
		zmqControlStatusTracker = mainPrometheusHandler->addNewStatTracker("zmq_control_server");
	else
		zmqControlStatusTracker = nullptr;

	while (loopFlag)
	{
		try
		{
			std::vector<zmq::message_t> recv_msgs;
			if (zmq::recv_multipart(socketRep, std::back_inserter(recv_msgs)))
			{
				if (zmqControlStatusTracker)
					zmqControlStatusTracker->incrementActive();
				if (zmqControlPerformanceTracker)
					zmqControlPerformanceTracker->startTimer();
				int reply = ZMQ_EVENT_HANDSHAKE_FAILED_NO_DETAIL;
				std::string replyBody = "";
				switch (*((uint64_t *)recv_msgs[0].data()))
				{
				case LOG_LEVEL_ID: {
					if (recv_msgs.size() == 2)
					{
						spdlog::warn("Log level change request received");
						std::string receivedMsg = std::string((char *)recv_msgs[1].data(), recv_msgs[1].size());

						if (!receivedMsg.compare("v"))
							spdlog::set_level(spdlog::level::info);
						if (!receivedMsg.compare("vv"))
							spdlog::set_level(spdlog::level::debug);
						if (!receivedMsg.compare("vvv"))
							spdlog::set_level(spdlog::level::trace);
						reply = ZMQ_EVENT_HANDSHAKE_SUCCEEDED;
					}
					else
						spdlog::error("Receive unknown number of messages for log level change");
					break;
				}
				/* ############################# MAKE MODIFICATIONS HERE ############################# */

				/* ################################ END MODIFICATIONS ################################ */
				default:
					spdlog::error("Unknown command received from control");
					break;
				}

				// Update status
				if (zmqControlStatusTracker)
				{
					if (reply == ZMQ_EVENT_HANDSHAKE_SUCCEEDED)
						zmqControlStatusTracker->incrementSuccess();
					else
						zmqControlStatusTracker->incrementFail();
				}

				// Send reply
				socketRep.send(zmq::const_buffer(&reply, sizeof(reply)), zmq::send_flags::sndmore);
				socketRep.send(zmq::const_buffer(replyBody.c_str(), replyBody.size()));
				if (zmqControlPerformanceTracker)
					zmqControlPerformanceTracker->endTimer();
			}
			else
				spdlog::trace("Controller ZMQ receive timeout");
		}
		catch (const std::exception &e)
		{
			spdlog::error("ZMQ {}", e.what());
		}

		if (curl && (alarmCtr - oldCtr) > HEARTBEAT_INTERVAL)
		{
			CURLcode retCode = curl_easy_perform(curl);
			if (retCode != CURLE_OK)
				spdlog::info("Heartbeat failed: {}", curl_easy_strerror(retCode));
			oldCtr = alarmCtr;
		}
	}

	// Cleanup
	spdlog::debug("Cleaning ZMQ control thread ...");
	socketRep.unbind(hostAddrRep);
	socketRep.close();

	curl_easy_cleanup(curl);

	spdlog::debug("ZMQ Control thread done");
}
// GCOVR_EXCL_STOP
