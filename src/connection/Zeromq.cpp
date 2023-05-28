#include "connection/Zeromq.hpp"
#include "Utils.hpp"

#include <fstream>

#include <sodium.h>
#include <spdlog/spdlog.h>
#include <zmq_addon.hpp>

constexpr int secretKeyLengthEncoded = 40; // ZeroMQ secret key length
constexpr int secretKeyLengthDecoded = 32; // ZeroMQ secret key length

void ZeroMQ::init(const std::shared_ptr<zmq::context_t> &ctx, const zmq::socket_type &type, const std::string &addr,
				  bool isBind, const std::string &sealedSecretPath)
{
	contextPtr = ctx;
	socketAddr = addr;
	isBinded = isBind;
	isActive = false;

	// Init ZMQ connection
	socketPtr = std::make_unique<zmq::socket_t>(*contextPtr, type);
	socketPtr->set(zmq::sockopt::linger, 0);
	socketPtr->set(zmq::sockopt::sndtimeo, 1000);
	socketPtr->set(zmq::sockopt::rcvtimeo, 1000);
	socketPtr->set(zmq::sockopt::heartbeat_ivl, 1000);
	socketPtr->set(zmq::sockopt::heartbeat_ttl, 3000);
	socketPtr->set(zmq::sockopt::heartbeat_timeout, 3000);

	if (!sealedSecretPath.empty())
	{
		// Read encrypted data from sealed box
		std::ifstream sealedBox(sealedSecretPath, std::ios_base::binary);
		if (!sealedBox.is_open())
		{
			throw std::runtime_error("Can't open the sealed box: Check file exists " + sealedSecretPath);
		}

		size_t curRead = 0;
		size_t maxRead = 2 * 1024 * 1024; // Approx 2 MB
		std::vector<unsigned char> encryptedData;
		std::copy_if(std::istream_iterator<unsigned char>(sealedBox), std::istream_iterator<unsigned char>(),
					 std::back_inserter(encryptedData), [&](const unsigned char &) { return (++curRead < maxRead); });

		if (curRead >= maxRead)
		{
			throw std::runtime_error("Read buffer reached its maximum (" + std::to_string(maxRead) +
									 " bytes). Check the sealed box for suspicious redirections (eg symlinks) " +
									 sealedSecretPath);
		}

		// Read plaintext variables
		auto sealedBoxPasswd = getEnvVar("CURVE_SEALED_BOX_PASSWD");
		if (sealedBoxPasswd.empty())
		{
			throw std::runtime_error("Missing environment variable: CURVE_SEALED_BOX_PASSWD should be provided");
		}

		// Prepare sensitive data buffers
		auto *passwdHash = static_cast<unsigned char *>(sodium_malloc(crypto_box_SECRETKEYBYTES));
		auto *secretKeyBuffer = static_cast<unsigned char *>(sodium_malloc(secretKeyLengthEncoded));
		auto *secretKeyEncodedBuffer = static_cast<unsigned char *>(sodium_malloc(secretKeyLengthDecoded));
		if (secretKeyBuffer == nullptr || passwdHash == nullptr || secretKeyEncodedBuffer == nullptr)
		{
			sodium_free(passwdHash);
			sodium_free(secretKeyBuffer);
			sodium_free(secretKeyEncodedBuffer);
			throw std::runtime_error("Memory operations failed for ZeroMQ server: " + getErrnoString(errno));
		}

		std::array<unsigned char, crypto_pwhash_SALTBYTES> salt{};
		// randombytes_buf_deterministic(
		// 	salt.data(), crypto_pwhash_SALTBYTES,
		// 	reinterpret_cast<const unsigned char *>(get_version().substr(get_version().size() - 32, 32).data()));

		if (crypto_pwhash(passwdHash, crypto_box_SECRETKEYBYTES, sealedBoxPasswd.data(), sealedBoxPasswd.size(),
						  salt.data(), crypto_pwhash_OPSLIMIT_SENSITIVE, crypto_pwhash_MEMLIMIT_SENSITIVE,
						  crypto_pwhash_ALG_DEFAULT) != 0 ||
			sodium_mprotect_readonly(passwdHash) != 0)
		{
			throw std::runtime_error("Internal error for ZeroMQ server");
		}

		if (crypto_box_seal_open(secretKeyBuffer, &encryptedData[crypto_box_SEALBYTES],
								 encryptedData.size() - crypto_box_SEALBYTES, encryptedData.data(), passwdHash) != 0 ||
			sodium_mprotect_readonly(secretKeyBuffer) != 0)
		{
			sodium_mprotect_readwrite(passwdHash);
			sodium_free(passwdHash);
			sodium_free(secretKeyBuffer);
			sodium_free(secretKeyEncodedBuffer);
			throw std::runtime_error("Secret key is corrupted or modified for ZeroMQ server");
		}

		// Set secretkey
		bool failed = false;
		std::string failureString;
		try
		{
			socketPtr->set(zmq::sockopt::curve_server, true);
			socketPtr->set(zmq::sockopt::curve_secretkey,
						   reinterpret_cast<char *>(
							   zmq_z85_decode(secretKeyEncodedBuffer, reinterpret_cast<char *>(secretKeyBuffer))));
		}
		catch (const std::exception &e)
		{
			failed = true;
			failureString = e.what();
		}

		// Clear buffers
		sodium_mprotect_readwrite(passwdHash);
		sodium_mprotect_readwrite(secretKeyBuffer);

		sodium_free(passwdHash);
		sodium_free(secretKeyBuffer);
		sodium_free(secretKeyEncodedBuffer);

		if (failed)
		{
			throw std::runtime_error("Can't set encryption keys: " + failureString);
		}
	}
}

ZeroMQ::ZeroMQ(const zmq::socket_type &type, const std::string &addr, bool isBind, const std::string &sealedSecretPath)
{
	contextPtr = std::make_shared<zmq::context_t>(1);
	init(contextPtr, type, addr, isBind, sealedSecretPath);
}

ZeroMQ::ZeroMQ(const std::shared_ptr<zmq::context_t> &ctx, const zmq::socket_type &type, const std::string &addr,
			   bool isBind, const std::string &sealedSecretPath)
{
	init(ctx, type, addr, isBind, sealedSecretPath);
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

size_t ZeroMQ::sendMessages(const std::vector<zmq::const_buffer> &msg)
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

ZeroMQ::~ZeroMQ() { stop(); }
