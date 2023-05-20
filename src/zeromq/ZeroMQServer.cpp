#include "zeromq/ZeroMQServer.hpp"

#include "Utils.hpp"

#include <fstream>

#include <sodium.h>
#include <spdlog/spdlog.h>

constexpr int secretKeyLength = 32; // ZeroMQ secret key length

constexpr uint32_t LOG_LEVEL_ID = (('L') | ('O' << 8) | ('G' << 16) | ('L' << 24));
constexpr uint32_t VERSION_INFO_ID = (('V') | ('E' << 8) | ('R' << 16) | ('I' << 24));
/* ################################################################################### */
/* ############################# MAKE MODIFICATIONS HERE ############################# */
/* ################################################################################### */

/* ################################################################################### */
/* ################################ END MODIFICATIONS ################################ */
/* ################################################################################### */

bool ZeroMQServer::initialise(const std::string &hostAddr, const std::shared_ptr<prometheus::Registry> &reg,
							  bool encryptMessages, const std::string &sealedSecretPath)
{
	if (m_initialised)
	{
		return false;
	}

	serverAddr = hostAddr;
	connectionPtr = std::make_unique<ZeroMQ>(zmq::socket_type::rep, serverAddr, true);

	connectionPtr->getSocket()->set(zmq::sockopt::sndtimeo, 1000);
	connectionPtr->getSocket()->set(zmq::sockopt::rcvtimeo, 50);

	if (encryptMessages)
	{
		// Read encrypted data from sealed box
		std::ifstream sealedBox(sealedSecretPath, std::ios_base::binary);
		if (!sealedBox.is_open())
		{
			spdlog::error("Can't open the sealed box: Check file exists {}", sealedSecretPath);
			return false;
		}

		size_t curRead = 0;
		size_t maxRead = 2 * 1024 * 1024;
		std::vector<unsigned char> encryptedData;
		std::copy_if(std::istream_iterator<unsigned char>(sealedBox), std::istream_iterator<unsigned char>(),
					 std::back_inserter(encryptedData), [&](const unsigned char &) { return (++curRead < maxRead); });

		if (curRead >= maxRead)
		{
			spdlog::error("Read buffer reached its maximum ({} bytes). Check the sealed box for suspicious "
						  "redirections (eg symlinks) {}",
						  maxRead, sealedSecretPath);
			return false;
		}

		// Read plaintext variables
		auto sealedBoxPasswd = getEnvVar("CURVE_SEALED_BOX_PASSWD");
		if (sealedBoxPasswd.empty())
		{
			spdlog::error("Missing environment variable: both of CURVE_SEALED_BOX_PASSWD should be provided");
			return false;
		}

		// Prepare sensitive data buffers
		auto *passwdHash = static_cast<unsigned char *>(sodium_malloc(crypto_box_SECRETKEYBYTES));
		auto *secretKeyBuffer = static_cast<unsigned char *>(sodium_malloc(secretKeyLength));
		if (secretKeyBuffer == nullptr || passwdHash == nullptr)
		{
			spdlog::error("Memory operations failed for ZeroMQ server: {}", getErrnoString(errno));
			sodium_free(passwdHash);
			sodium_free(secretKeyBuffer);
			return false;
		}

		std::array<unsigned char, crypto_pwhash_SALTBYTES> salt{};
		randombytes_buf_deterministic(
			salt.data(), crypto_pwhash_SALTBYTES,
			reinterpret_cast<const unsigned char *>(get_version().substr(get_version().size() - 32, 32).data()));

		if (crypto_pwhash(passwdHash, crypto_box_SECRETKEYBYTES, sealedBoxPasswd.data(), sealedBoxPasswd.size(),
						  salt.data(), crypto_pwhash_OPSLIMIT_SENSITIVE, crypto_pwhash_MEMLIMIT_SENSITIVE,
						  crypto_pwhash_ALG_DEFAULT) != 0 ||
			sodium_mprotect_readonly(passwdHash) != 0)
		{
			spdlog::error("Internal error for ZeroMQ server");
			sodium_free(passwdHash);
			sodium_free(secretKeyBuffer);
			return false;
		}

		std::array<unsigned char, crypto_box_PUBLICKEYBYTES> publicKey{};
		if (crypto_box_seal_open(secretKeyBuffer, encryptedData.data(), encryptedData.size(), publicKey.data(),
								 passwdHash) != 0 ||
			sodium_mprotect_readonly(secretKeyBuffer) != 0)
		{
			spdlog::error("Secret key is corrupted or modified for ZeroMQ server");
			sodium_mprotect_readwrite(passwdHash);
			sodium_free(passwdHash);
			sodium_free(secretKeyBuffer);
			return false;
		}

		// Set secretkey
		bool failed = false;
		try
		{
			connectionPtr->getSocket()->set(zmq::sockopt::curve_server, true);
			connectionPtr->getSocket()->set(zmq::sockopt::curve_secretkey, reinterpret_cast<char *>(secretKeyBuffer));
		}
		catch (const std::exception &e)
		{
			spdlog::error("Can't set encryption keys: {}", e.what());
			failed = true;
		}

		// Clear buffers
		sodium_mprotect_readwrite(passwdHash);
		sodium_mprotect_readwrite(secretKeyBuffer);

		sodium_free(passwdHash);
		sodium_free(secretKeyBuffer);

		if (failed)
		{
			return false;
		}
	}

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
		std::vector<zmq::const_buffer> replyMsgs;

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

bool ZeroMQServerMessageCallback(const std::vector<zmq::message_t> &recvMsgs, std::vector<zmq::const_buffer> &replyMsgs)
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
			spdlog::error("Receive unknown number of messages for log level change");
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
