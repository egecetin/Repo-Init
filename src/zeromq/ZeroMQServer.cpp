#include "zeromq/ZeroMQServer.hpp"

#include "Utils.hpp"

#include <spdlog/spdlog.h>

constexpr int SLEEP_INTERVAL_MS = 50;

constexpr uint32_t LOG_LEVEL_ID = ('L' | ('O' << 8) | ('G' << 16) | ('L' << 24));
constexpr uint32_t VERSION_INFO_ID = ('V' | ('E' << 8) | ('R' << 16) | ('I' << 24));
constexpr uint32_t PING_PONG_ID = ('P' | ('I' << 8) | ('N' << 16) | ('G' << 24));
constexpr uint32_t STATUS_CHECK_ID = ('S' | ('C' << 8) | ('H' << 16) | ('K' << 24));
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
	try
	{
		connectionPtr = std::make_unique<ZeroMQ>(zmq::socket_type::rep, serverAddr, true);
	}
	catch (const std::exception &e)
	{
		spdlog::error("Can't initialize ZeroMQ server: {}", e.what());
		return false;
	}

	connectionPtr->getSocket()->set(zmq::sockopt::rcvtimeo, SLEEP_INTERVAL_MS);

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
	auto recvMsgs = connectionPtr->recvMessages();

	if (!recvMsgs.empty())
	{
		std::vector<zmq::message_t> replyMsgs;

		ZeroMQServerStats serverStats;
		serverStats.processingTimeStart = std::chrono::high_resolution_clock::now();
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

bool ZeroMQServerMessageCallback(const std::vector<zmq::message_t> &recvMsgs, std::vector<zmq::message_t> &replyMsgs)
{
	spdlog::trace("Received {} messages", recvMsgs.size());
	replyMsgs.clear();

	std::string replyBody;
	int reply = ZMQ_EVENT_HANDSHAKE_FAILED_NO_DETAIL;
	switch (*(static_cast<const uint64_t *>(recvMsgs[0].data())))
	{
	case LOG_LEVEL_ID: {
		if (recvMsgs.size() != 2)
		{
			spdlog::error("Received unknown number of messages for log level change");
			break;
		}

		spdlog::warn("Log level change request received");
		const auto receivedMsg = std::string(static_cast<const char *>(recvMsgs[1].data()), recvMsgs[1].size());

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
			spdlog::error("Received unknown number of messages for version information");
			break;
		}

		reply = ZMQ_EVENT_HANDSHAKE_SUCCEEDED;
		replyBody = get_version();
		break;
	}
	case PING_PONG_ID: {
		if (recvMsgs.size() != 1)
		{
			spdlog::error("Received unknown number of messages for ping");
			break;
		}

		reply = ZMQ_EVENT_HANDSHAKE_SUCCEEDED;
		replyBody = "PONG";
		break;
	}
	case STATUS_CHECK_ID: {
		if (recvMsgs.size() != 1)
		{
			spdlog::error("Received unknown number of messages for status check");
			break;
		}

		reply = ZMQ_EVENT_HANDSHAKE_SUCCEEDED;

		std::ostringstream oss;
		oss << "{";
		for (const auto &entry : vCheckFlag)
		{
			oss << "\"" << entry.first << "\":" << (entry.second->_M_i ? "1," : "0,");
		}
		replyBody = oss.str();
		replyBody.replace(replyBody.size() - 1, 1, "}");
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
