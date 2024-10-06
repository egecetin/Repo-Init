#include "zeromq/ZeroMQAuth.hpp"

#include "zeromq/ZeroMQStats.hpp"

#include <spdlog/spdlog.h>

class ZeroMQAuthException : public std::exception {
  private:
	std::string msg_;

  public:
	explicit ZeroMQAuthException(const std::string &message) : msg_(message) {}
	virtual const char *what() const noexcept override { return msg_.c_str(); }
};

bool ZeroMQAuth::authenticateConnection(const std::vector<zmq::message_t> &recvMsgs,
										std::vector<zmq::message_t> &replyMsgs)
{
	spdlog::trace("Received {} authentication messages", recvMsgs.size());
	replyMsgs.clear();

	std::string statusCode = "500";
	std::string statusText = "";
	try
	{
		// Ensure that at least 6 messages are received
		if (recvMsgs.size() < 6)
		{
			throw ZeroMQAuthException("Received unknown number of messages for authentication");
		}
        
        // Parse received messages
        const auto versionStr = std::string(static_cast<const char *>(recvMsgs[0].data()), recvMsgs[0].size());
		const auto domainStr = std::string(static_cast<const char *>(recvMsgs[2].data()), recvMsgs[2].size());
		const auto addressStr = std::string(static_cast<const char *>(recvMsgs[3].data()), recvMsgs[3].size());
		const auto identityStr = std::string(static_cast<const char *>(recvMsgs[4].data()), recvMsgs[4].size());
		const auto mechanismStr = std::string(static_cast<const char *>(recvMsgs[5].data()), recvMsgs[5].size());

        spdlog::debug("Received authentication request for {} ({})", identityStr, addressStr);

		// Check version
		if (versionStr.compare("1.0"))
		{
			throw ZeroMQAuthException("Unsupported authentication version for " + identityStr + " (" + addressStr + "): " + versionStr);
		}

        // Check domain
        if (!checkDomainAllowed(domainStr))
        {
            throw ZeroMQAuthException("Domain not allowed for " + identityStr + " (" + addressStr + "): " + domainStr);
        }
        spdlog::debug("Domain allowed for {} ({})", identityStr, addressStr);

        // Check address
        if (!checkAddressAllowed(addressStr))
        {
            throw ZeroMQAuthException("Address not allowed for " + identityStr + " (" + addressStr + "): " + addressStr);
        }
        spdlog::debug("Address allowed for {} ({})", identityStr, addressStr);

        // Check mechanism
        if (!checkMechanismAllowed(mechanismStr))
        {
            throw ZeroMQAuthException("Mechanism not allowed for " + identityStr + " (" + addressStr + "): " + mechanismStr);
        }
        spdlog::debug("Mechanism allowed for {} ({})", identityStr, addressStr);

        switch (constHasher(mechanismStr.c_str()))
        {
        case constHasher("NULL"):
            spdlog::info("Authenticated NULL for {} ({})", identityStr, addressStr);
            break;
        case constHasher("PLAIN"):
            /* code */
            break;
        case constHasher("CURVE"):
            /* code */
            break;
        
        default:
            // This should not happen since we already checked the mechanism
            throw std::runtime_error("Mechanism calculation failed");
        }

        spdlog::debug("Authenticated {} ({})", identityStr, addressStr);
	}
	catch (const ZeroMQAuthException &e)
	{
		statusCode = "400";
		statusText = e.what();
        spdlog::warn("Authentication failed: {}", e.what());
	}
	catch (const std::exception &e)
	{
		statusText = e.what();
	}

	// Prepare reply
	replyMsgs.emplace_back("1.0");
	replyMsgs.emplace_back(recvMsgs[1]);
	replyMsgs.emplace_back(statusCode);
	replyMsgs.emplace_back(statusText);

	return statusCode == "200";
}

void ZeroMQAuth::update()
{
	auto recvMsgs = recvMessages();

	if (!recvMsgs.empty())
	{
		std::vector<zmq::message_t> replyMsgs;

		ZeroMQServerStats serverStats;
		auto processingTimeStart = std::chrono::high_resolution_clock::now();
		serverStats.isSuccessful = authenticateConnection(recvMsgs, replyMsgs);

		size_t nSentMsg = sendMessages(replyMsgs);
		if (nSentMsg != replyMsgs.size())
		{
			spdlog::warn("Can't send whole reply: Sent messages {} / {}", nSentMsg, replyMsgs.size());
		}
		serverStats.processingTime = std::chrono::high_resolution_clock::now() - processingTimeStart;

		if (_stats)
		{
			_stats->consumeStats(recvMsgs, replyMsgs, serverStats);
		}
	}
}

void ZeroMQAuth::threadFunc()
{
	spdlog::info("ZeroMQ auth started");
	while (!_shouldStop._M_i)
	{
		try
		{
			update();
		}
		catch (const std::exception &e)
		{
			spdlog::error("ZeroMQ auth failed: {}", e.what());
		}
	}
	spdlog::info("ZeroMQ auth stopped");
}

bool ZeroMQAuth::startAuth()
{
	_shouldStop.clear();

	if (start())
	{
		_authThread = std::make_unique<std::thread>(&ZeroMQAuth::threadFunc, this);
		return true;
	}
	return false;
}

void ZeroMQAuth::stopAuth()
{
	_shouldStop.test_and_set();
	if (_authThread && _authThread->joinable())
	{
		_authThread->join();
		_authThread.reset();
	}

	stop();
}
