#include "Control.hpp"
#include "Utils.hpp"
#include "Version.h"
#include "connection/Http.hpp"
#include "connection/Zeromq.hpp"
#include "metrics/Performance.hpp"
#include "metrics/Reporter.hpp"
#include "metrics/Status.hpp"
#include "telnet/TelnetServer.hpp"

#include <chrono>
#include <thread>

#include <spdlog/spdlog.h>

// GCOVR_EXCL_START
void telnetControlThread()
{
	// Init Telnet Server
	auto telnetServerPtr = std::make_shared<TelnetServer>();

	// Init performance tracker if prometheus enabled
	std::shared_ptr<PerformanceTracker> telnetPerformanceTracker(nullptr);
	if (mainPrometheusHandler)
		telnetPerformanceTracker = mainPrometheusHandler->addNewPerfTracker("telnet_server", 1000);
	std::shared_ptr<StatusTracker> telnetStatusTracker(nullptr);
	if (mainPrometheusHandler)
		telnetStatusTracker = mainPrometheusHandler->addNewStatTracker("telnet_server");

	if (TELNET_PORT && telnetServerPtr->initialise(TELNET_PORT, "> ", telnetStatusTracker))
	{
		telnetServerPtr->connectedCallback(TelnetConnectedCallback);
		telnetServerPtr->newLineCallback(TelnetMessageCallback);
		telnetServerPtr->tabCallback(TelnetTabCallback);
		spdlog::info("Telnet server created at {}", TELNET_PORT);
	}
	else
		return;

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
	// Init ZMQ connection
	std::unique_ptr<ZeroMQ> zmqContext(nullptr);
	std::string hostAddrRep = CONTROL_IPC_PATH + "/" + PROJECT_NAME;
	try
	{
		zmqContext = std::make_unique<ZeroMQ>(zmq::socket_type::rep, hostAddrRep, true);
		if (!zmqContext->start())
			throw std::runtime_error("Can't init ZMQ control channel");
	}
	catch (const std::exception &e)
	{
		spdlog::critical("Can't bind to {}: {}", hostAddrRep, e.what());
		loopFlag = false;
	}

	// Prepare heartbeat
	size_t oldCtr = alarmCtr;
	std::unique_ptr<HTTP> heartBeat(nullptr);
	std::string HEARTBEAT_ADDRESS = readSingleConfig(CONFIG_FILE_PATH, "HEARTBEAT_ADDRESS");
	try
	{
		if (!HEARTBEAT_ADDRESS.empty())
			heartBeat = std::make_unique<HTTP>(HEARTBEAT_ADDRESS, 100);
	}
	catch (const std::exception &e)
	{
		spdlog::error("Can't init heartbeat handler: {}", e.what());
		loopFlag = false;
	}

	// Init performance and status tracker if prometheus enabled
	std::shared_ptr<PerformanceTracker> zmqControlPerformanceTracker(nullptr);
	if (mainPrometheusHandler)
		zmqControlPerformanceTracker = mainPrometheusHandler->addNewPerfTracker("zmq_control_server", 1000);
	std::shared_ptr<StatusTracker> zmqControlStatusTracker(nullptr);
	if (mainPrometheusHandler)
		zmqControlStatusTracker = mainPrometheusHandler->addNewStatTracker("zmq_control_server");

	while (loopFlag)
	{
		// ZeroMQ
		try
		{
			std::vector<zmq::message_t> recv_msgs = zmqContext->recvMessages();
			if (!recv_msgs.empty())
			{
				if (zmqControlStatusTracker)
					zmqControlStatusTracker->incrementActive();
				if (zmqControlPerformanceTracker)
					zmqControlPerformanceTracker->startTimer();

				std::string replyBody = "";
				int reply = ZMQ_EVENT_HANDSHAKE_FAILED_NO_DETAIL;
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

				// Prepare reply
				std::vector<zmq::const_buffer> replyMessages;
				replyMessages.push_back(zmq::const_buffer(&reply, sizeof(reply)));
				replyMessages.push_back(zmq::const_buffer(replyBody.c_str(), replyBody.size()));

				size_t nSentMsg = zmqContext->sendMessages(replyMessages);
				if (nSentMsg != replyMessages.size())
					spdlog::warn("Can't send whole reply: Sent messages {} / {}", nSentMsg, replyMessages.size());

				if (zmqControlPerformanceTracker)
					zmqControlPerformanceTracker->endTimer();
			}
			else
				spdlog::trace("Controller ZMQ receive timeout");
		}
		catch (const std::exception &e)
		{
			spdlog::error("ZMQ failed: {}", e.what());
		}

		// Heartbeat
		try
		{
			if (heartBeat && (alarmCtr - oldCtr) > HEARTBEAT_INTERVAL)
			{
				HttpStatus::Code statusCode = HttpStatus::Code::xxx_max;
				std::string recvPayload;

				CURLcode retCode = heartBeat->sendPOSTRequest("", "", recvPayload, statusCode);
				if (retCode != CURLE_OK)
					spdlog::warn("Heartbeat failed: {}", curl_easy_strerror(retCode));
				if (statusCode != HttpStatus::Code::OK)
					spdlog::warn("Heartbeat failed: {}", HttpStatus::reasonPhrase(statusCode));
				oldCtr = alarmCtr;
			}
		}
		catch (const std::exception &e)
		{
			spdlog::error("Heartbeat failed: {}", e.what());
		}
	}

	// Cleanup
	spdlog::debug("Cleaning ZMQ control thread ...");

	spdlog::debug("ZMQ Control thread done");
}
// GCOVR_EXCL_STOP
