#include "connection/Zeromq.hpp"
#include "Utils.hpp"

#include <spdlog/spdlog.h>
#include <zmq_addon.hpp>

// ZeroMQ send/receive timeouts in milliseconds
constexpr int ZEROMQ_MSG_TIMEOUT_MS = 1000;
// ZeroMQ heartbeat timeout in milliseconds
constexpr int ZEROMQ_HEARTBEAT_TIMEOUT_MS = 1000;

void ZeroMQ::init(const std::shared_ptr<zmq::context_t> &ctx, const zmq::socket_type &type, const std::string &addr,
				  bool isBind)
{
	contextPtr = ctx;
	socketAddr = addr;
	isBinded = isBind;
	isActive = false;

	// Init ZMQ connection
	socketPtr = std::make_unique<zmq::socket_t>(*contextPtr, type);
	socketPtr->set(zmq::sockopt::linger, 0);
	socketPtr->set(zmq::sockopt::sndtimeo, ZEROMQ_MSG_TIMEOUT_MS);
	socketPtr->set(zmq::sockopt::rcvtimeo, ZEROMQ_MSG_TIMEOUT_MS);
	socketPtr->set(zmq::sockopt::heartbeat_ivl, ZEROMQ_HEARTBEAT_TIMEOUT_MS);
	socketPtr->set(zmq::sockopt::heartbeat_ttl, ZEROMQ_HEARTBEAT_TIMEOUT_MS * 3);
	socketPtr->set(zmq::sockopt::heartbeat_timeout, ZEROMQ_HEARTBEAT_TIMEOUT_MS);
}

ZeroMQ::ZeroMQ(const zmq::socket_type &type, const std::string &addr, bool isBind)
	: contextPtr(std::make_shared<zmq::context_t>(1))
{
	init(contextPtr, type, addr, isBind);
}

ZeroMQ::ZeroMQ(const std::shared_ptr<zmq::context_t> &ctx, const zmq::socket_type &type, const std::string &addr,
			   bool isBind)
{
	init(ctx, type, addr, isBind);
}

bool ZeroMQ::start()
{
	if (isActive)
	{
		return false;
	}

	if (isBinded)
	{
		socketPtr->bind(socketAddr);
	}
	else
	{
		socketPtr->connect(socketAddr);
	}
	isActive = true;

	return true;
}

void ZeroMQ::stop()
{
	if (!isActive)
	{
		return;
	}

	if (isBinded)
	{
		socketPtr->unbind(socketAddr);
	}
	else
	{
		socketPtr->disconnect(socketAddr);
	}
	isActive = false;
}

std::vector<zmq::message_t> ZeroMQ::recvMessages()
{
	std::vector<zmq::message_t> recvMsgs;
	if (!isActive)
	{
		spdlog::warn("Connection needs to starting");
	}
	else
	{
		zmq::recv_multipart(*socketPtr, std::back_inserter(recvMsgs));
	}
	return recvMsgs;
}

size_t ZeroMQ::sendMessages(std::vector<zmq::message_t> &msg)
{
	zmq::send_result_t res;
	if (!isActive)
	{
		spdlog::warn("Connection needs to starting");
	}
	else
	{
		res = zmq::send_multipart(*socketPtr, msg);
	}

	if (res.has_value())
	{
		return res.value();
	}
	return 0;
}

ZeroMQ::~ZeroMQ()
{
	try
	{
		stop();
	}
	catch (const std::exception &)
	{
		// Just catch the exception to ensure not throwing to main code
	}
}
