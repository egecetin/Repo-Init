#include "Control.h"

#include <chrono>
#include <thread>

#include <curl/curl.h>
#include <spdlog/spdlog.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

size_t writeData(void *, size_t size, size_t nmemb, void *) { return size * nmemb; }

constexpr size_t constHasher(const char *s, size_t index = 0)
{
	return s + index == nullptr || s[index] == '\0' ? 55 : constHasher(s, index + 1) * 33 + (unsigned char)(s[index]);
}

std::vector<std::pair<std::string, std::string>> telnetCommands = {
	{"help", "Prints available commands"},
	{"disable log", "Resets logger level"},
	{"enable log", "Enable specified logger level. Level can be \"v\" (info), \"vv\" (debug) and \"vvv\" (trace)"},
	{"quit", "Ends the connection"}};

void TelnetPrintAvailableCommands(SP_TelnetSession session)
{
	// Print available commands
	session->sendLine("");
	session->sendLine("Available commands:");
	session->sendLine("");
	for (const auto &entry : telnetCommands)
	{
		char buffer[BUFSIZ] = {'\0'};
		std::snprintf(buffer, BUFSIZ, "%-25s : %s", entry.first.c_str(), entry.second.c_str());
		session->sendLine(buffer);
	}
}

void TelnetConnectedCallback(SP_TelnetSession session)
{
	session->sendLine("\r\n"
					  "𝑲𝒆𝒆𝒑 𝒚𝒐𝒖𝒓 𝒆𝒚𝒆𝒔 𝒐𝒏 𝒕𝒉𝒆 𝒔𝒕𝒂𝒓𝒔 "
					  "𝒂𝒏𝒅 𝒚𝒐𝒖𝒓 𝒇𝒆𝒆𝒕 𝒐𝒏 𝒕𝒉𝒆 𝒈𝒓𝒐𝒖𝒏𝒅 "
					  "\r\n");
	TelnetPrintAvailableCommands(session);
}

void TelnetMessageCallback(SP_TelnetSession session, std::string line)
{
	spdlog::trace("Received message {}", line);

	// Send received message for user terminal
	session->sendLine(line);

	// Process received message
	switch (constHasher(line.c_str()))
	{
	case constHasher("Test Message"):
		session->sendLine("OK");
		break;
	case constHasher("help"):
		TelnetPrintAvailableCommands(session);
		break;
	case constHasher("disable log"):
		session->sendLine("Default log mode enabled");
		spdlog::set_level(spdlog::level::info);
		break;
	case constHasher("disable log all"): // Internal use only
		session->sendLine("Disabling all logs");
		spdlog::set_level(spdlog::level::off);
		break;
	case constHasher("enable log v"):
		session->sendLine("Info log mode enabled");
		spdlog::set_level(spdlog::level::info);
		break;
	case constHasher("enable log vv"):
		session->sendLine("Debug log mode enabled");
		spdlog::set_level(spdlog::level::debug);
		break;
	case constHasher("enable log vvv"):
		session->sendLine("Trace log mode enabled");
		spdlog::set_level(spdlog::level::trace);
		break;
	case constHasher("quit"):
		session->sendLine("Closing connection");
		session->sendLine("Goodbye!");
		session->markTimeout();
		break;
	default:
		session->sendLine("Unknown command received");
		break;
	}
}

std::string TelnetTabCallback(SP_TelnetSession session, std::string line)
{
	std::string retval = "";

	size_t ctr = 0;
	std::stringstream ss;
	for (const auto &entry : telnetCommands)
	{
		if (entry.first.rfind(line, 0) == 0)
		{
			++ctr;
			retval = entry.first;
			ss << entry.first << std::setw(25);
		}
	}
	// Send suggestions if found any. If there is only one command retval will invoke completion
	if (ctr != 1 && ss.str().size())
	{
		session->sendLine(ss.str());
		retval = "";
	}

	return retval;
}

// GCOVR_EXCL_START
void telnetControlThread()
{
	bool tryAgain = false;

	// Init Telnet Server
	auto telnetServerPtr = std::make_shared<TelnetServer>();

start:
	if (tryAgain)
		TELNET_PORT = std::stoi(readSingleConfig(CONFIG_FILE_PATH, "TELNET_PORT"));
	if (telnetServerPtr->initialise(TELNET_PORT, "> "))
	{
		telnetServerPtr->connectedCallback(TelnetConnectedCallback);
		telnetServerPtr->newLineCallback(TelnetMessageCallback);
		telnetServerPtr->tabCallback(TelnetTabCallback);
		spdlog::info("Telnet server created at {}", TELNET_PORT);
	}
	else
	{
		tryAgain = true;
		std::this_thread::sleep_for(std::chrono::seconds(30));
		goto start;
	}

	while (loopFlag)
	{
		try
		{
			// Update Telnet connection
			telnetServerPtr->update();
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

	std::string hostAddrRep = CONTROL_IPC_PATH + "/XXX";
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
	std::string heartbeatAddr = readSingleConfig(CONFIG_FILE_PATH, "HEARTBEAT_ADDRESS");
	if (!heartbeatAddr.empty())
	{
		curl = curl_easy_init();
		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_URL, heartbeatAddr.c_str());
			curl_easy_setopt(curl, CURLOPT_POST, 1L);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 100);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
		}
	}

	while (loopFlag)
	{
		try
		{
			std::vector<zmq::message_t> recv_msgs;
			if (zmq::recv_multipart(socketRep, std::back_inserter(recv_msgs)))
			{
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
				default:
					spdlog::error("Unknown command received from control");
					break;
				}

				// Send reply
				socketRep.send(zmq::const_buffer(&reply, sizeof(reply)), zmq::send_flags::sndmore);
				socketRep.send(zmq::const_buffer(replyBody.c_str(), replyBody.size()));
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
			curl_easy_perform(curl);
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
