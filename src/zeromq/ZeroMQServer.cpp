#include "zeromq/ZeroMQServer.hpp"

#include "Utils.hpp"

#include <spdlog/spdlog.h>

constexpr uint32_t LOG_LEVEL_ID = (('L') | ('O' << 8) | ('G' << 16) | ('L' << 24));
constexpr uint32_t VERSION_INFO_ID = (('V') | ('E' << 8) | ('R' << 16) | ('I' << 24));
/* ################################################################################### */
/* ############################# MAKE MODIFICATIONS HERE ############################# */
/* ################################################################################### */

/* ################################################################################### */
/* ################################ END MODIFICATIONS ################################ */
/* ################################################################################### */

bool ZeroMQServer::initialise(const std::string &hostAddr, const std::shared_ptr<prometheus::Registry> &reg)
{
	if (m_initialised)
	{
		return false;
	}

	serverAddr = hostAddr;
	connectionPtr = std::make_unique<ZeroMQ>(zmq::socket_type::rep, serverAddr, true);

	connectionPtr->getSocket()->set(zmq::sockopt::sndtimeo, 1000);
	connectionPtr->getSocket()->set(zmq::sockopt::rcvtimeo, 50);

	if (connectionPtr->start())
	{
		// If prometheus registry is provided prepare statistics
		if (reg)
		{
			stats = std::make_unique<ZeroMQStats>(reg);
		}

		m_initialised = true;
		return true;
	}

	return false;
}

void ZeroMQServer::update()
{
	std::vector<zmq::const_buffer> replyMsgs;
	auto recvMsgs = connectionPtr->recvMessages();

	if (static_cast<int>(!recvMsgs.empty()) != 0)
	{
		ZeroMQServerStats serverStats;
		serverStats.processingTimeStart = std::chrono::high_resolution_clock::now();
		try
		{
			// NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.gets)
			spdlog::info("ZeroMQ control message received from {}", recvMsgs[0].gets("Peer-Address"));
		}
		catch (const std::exception &e)
		{
		}

		serverStats.isSuccessful = messageCallback() && messageCallback()(recvMsgs, replyMsgs);

		size_t nSentMsg = connectionPtr->sendMessages(replyMsgs);
		if (nSentMsg != replyMsgs.size())
		{
			spdlog::warn("Can't send whole reply: Sent messages {} / {}", nSentMsg, replyMsgs.size());
		}
		serverStats.processingTimeEnd = std::chrono::high_resolution_clock::now();

		if (stats)
		{
			stats->consumeStats(recvMsgs, replyMsgs, serverStats);
		}
	}
}

void ZeroMQServer::shutdown()
{
	if (!m_initialised)
	{
		return;
	}
	connectionPtr->stop();

	m_initialised = false;
}

bool ZeroMQServerMessageCallback(const std::vector<zmq::message_t> &recvMsgs, std::vector<zmq::const_buffer> &replyMsgs)
{
	spdlog::trace("Received {} messages", recvMsgs.size());
	replyMsgs.clear();

	std::string replyBody;
	int reply = ZMQ_EVENT_HANDSHAKE_FAILED_NO_DETAIL;
	switch (*((uint64_t *)recvMsgs[0].data()))
	{
	case LOG_LEVEL_ID: {
		if (recvMsgs.size() != 2)
		{
			spdlog::error("Receive unknown number of messages for log level change");
			break;
		}

		spdlog::warn("Log level change request received");
		std::string receivedMsg = std::string((char *)recvMsgs[1].data(), recvMsgs[1].size());

		if (receivedMsg == "v")
		{
			spdlog::set_level(spdlog::level::info);
		}
		if (receivedMsg == "vv")
		{
			spdlog::set_level(spdlog::level::debug);
		}
		if (receivedMsg == "vvv")
		{
			spdlog::set_level(spdlog::level::trace);
		}
		reply = ZMQ_EVENT_HANDSHAKE_SUCCEEDED;
		break;
	}
	case VERSION_INFO_ID: {
		if (recvMsgs.size() != 1)
		{
			spdlog::error("Receive unknown number of messages for version information");
			break;
		}

		reply = ZMQ_EVENT_HANDSHAKE_SUCCEEDED;
		replyBody = get_version();
		break;
	}
	/* ################################################################################### */
	/* ############################# MAKE MODIFICATIONS HERE ############################# */
	/* ################################################################################### */

	/* ################################################################################### */
	/* ################################ END MODIFICATIONS ################################ */
	/* ################################################################################### */
	default:
		spdlog::error("Unknown command received from control");
		break;
	}

	// Prepare reply
	replyMsgs.emplace_back(&reply, sizeof(reply));
	replyMsgs.emplace_back(replyBody.c_str(), replyBody.size());

	return reply == ZMQ_EVENT_HANDSHAKE_SUCCEEDED;
}
