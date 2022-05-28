#include "Control.h"

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

	std::string hostAddrRep = "ipc://" + CONTROL_IPC_PATH + "/XXX";
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
				spdlog::error("Unknown command recieved from control");
				break;
			}

			socketRep.send(zmq::const_buffer(&reply, sizeof(reply)));
		}
		else
			spdlog::debug("Controller receive timeout");

		// Print total bandwidth
		if (sigReadyFlag)
		{
			sigReadyFlag = false;
			spdlog::trace("Current bandwidth {}", bandwidth);
		}
	}

	// Cleanup
	spdlog::debug("Cleaning control thread ...");
	socketRep.unbind(hostAddrRep);
	socketRep.close();

	spdlog::debug("Control thread done");
}
