#include "Control.h"

#include <chrono>
#include <thread>

#include <spdlog/spdlog.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

constexpr size_t constHasher(const char *s, size_t index = 0)
{
	return s + index == nullptr || s[index] == '\0' ? 55 : constHasher(s, index + 1) * 33 + (unsigned char)(s[index]);
}

void TelnetConnectedCallback(SP_TelnetSession session)
{
	session->sendLine("\r\n"
					  "𝑲𝒆𝒆𝒑 𝒚𝒐𝒖𝒓 𝒆𝒚𝒆𝒔 𝒐𝒏 𝒕𝒉𝒆 𝒔𝒕𝒂𝒓𝒔 "
					  "𝒂𝒏𝒅 𝒚𝒐𝒖𝒓 𝒇𝒆𝒆𝒕 𝒐𝒏 𝒕𝒉𝒆 𝒈𝒓𝒐𝒖𝒏𝒅 "
					  "\r\n");

	// Print available commands
	session->sendLine("Available commands:");
	session->sendLine("");
	session->sendLine("quit         : Ends the connection");
}

void TelnetMessageCallback(SP_TelnetSession session, std::string line)
{
	// Send received message for user terminal
	session->sendLine(line);

	// Process received message
	switch (constHasher(line.c_str()))
	{
	case constHasher("Test Message"):
		session->sendLine("OK");
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

// GCOVR_EXCL_START
void telnetControlThread()
{
	// Init Telnet Server
	auto telnetServerPtr = std::make_shared<TelnetServer>();
	telnetServerPtr->initialise(TELNET_PORT, "> ");
	telnetServerPtr->connectedCallback(TelnetConnectedCallback);
	telnetServerPtr->newLineCallback(TelnetMessageCallback);
	spdlog::debug("Telnet server created at {}", TELNET_PORT);

	while (loopFlag)
	{
		// Update Telnet connection
		telnetServerPtr->update();
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

	while (loopFlag)
	{
		std::vector<zmq::message_t> recv_msgs;
		if (zmq::recv_multipart(socketRep, std::back_inserter(recv_msgs)))
		{
			int reply = ZMQ_EVENT_HANDSHAKE_FAILED_NO_DETAIL;
			switch (*((uint64_t *)recv_msgs[0].data()))
			{
			case LOG_LEVEL_ID: {
				if (recv_msgs.size() == 2)
				{
					spdlog::warn("Log level change request received");
					std::string receivedMsg = std::string((char *)recv_msgs[1].data(), recv_msgs[1].size());

					if (!receivedMsg.compare("-v"))
						spdlog::set_level(spdlog::level::info);
					if (!receivedMsg.compare("-vv"))
						spdlog::set_level(spdlog::level::debug);
					if (!receivedMsg.compare("-vvv"))
						spdlog::set_level(spdlog::level::trace);
				}
				else
					spdlog::error("Receive unknown number of messages for log level change");

				break;
			}
			default:
				spdlog::error("Unknown command received from control");
				break;
			}

			socketRep.send(zmq::const_buffer(&reply, sizeof(reply)));
		}
		else
			spdlog::trace("Controller ZMQ receive timeout");
	}

	// Cleanup
	spdlog::debug("Cleaning ZMQ control thread ...");
	socketRep.unbind(hostAddrRep);
	socketRep.close();

	spdlog::debug("ZMQ Control thread done");
}
// GCOVR_EXCL_STOP
