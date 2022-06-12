#include "Control.h"

void TelnetConnectedCallback(SP_TelnetSession session)
{
	spdlog::info("New connection received");
	session->sendLine("Wellcome!");
}

void TelnetMessageCallback(SP_TelnetSession session, std::string line)
{
	spdlog::trace("Received message {}", line);
	session->sendLine("Copy that.");
}

// GCOVR_EXCL_START
void controllerThread()
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

	// Init Telnet Server
	auto telnetServerPtr = std::make_shared<TelnetServer>();
	telnetServerPtr->initialise(TELNET_PORT);
	telnetServerPtr->connectedCallback(TelnetConnectedCallback);
	telnetServerPtr->newLineCallback(TelnetMessageCallback);
	spdlog::debug("Telnet server created at {}", TELNET_PORT);

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

		// Update Telnet connection
		telnetServerPtr->update();
	}

	// Cleanup
	spdlog::debug("Cleaning control thread ...");
	socketRep.unbind(hostAddrRep);
	socketRep.close();
	telnetServerPtr->shutdown();

	spdlog::debug("Control thread done");
}
// GCOVR_EXCL_STOP
