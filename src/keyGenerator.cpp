#include "Utils.hpp"

#include <fstream>

#include <sodium.h>
#include <spdlog/spdlog.h>
#include <zmq.hpp>

int main(int argc, char **argv)
{
	InputParser input(argc, argv);

	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [Key Generator] [%^%l%$] : %v");
	if (!input.cmdOptionExists("--password") || input.getCmdOption("--password").empty())
	{
		spdlog::error("Password should be provided with --password option");
		return EXIT_FAILURE;
	}
	if (!input.cmdOptionExists("--output") || input.getCmdOption("--output").empty())
	{
		spdlog::error("Output path should be provided with --output option");
		return EXIT_FAILURE;
	}

	// Initialize
	auto password = input.getCmdOption("--password");
	auto outputPath = input.getCmdOption("--output");

	if (sodium_init() < 0)
	{
		spdlog::critical("Can't init cryptographic library");
		return EXIT_FAILURE;
	}

	// Generate ZeroMQ certificate
	std::array<char, 40> z85_pk{};
	std::array<char, 40> z85_sk{};
	if (zmq_curve_keypair(z85_pk.data(), z85_sk.data()) != 0)
	{
		spdlog::error("Can't generate ZeroMQ key pairs");
		return EXIT_FAILURE;
	}

	// Generate sealed box keys
	std::array<unsigned char, crypto_pwhash_SALTBYTES> salt{};
	// randombytes_buf_deterministic(
	// 	salt.data(), crypto_pwhash_SALTBYTES,
	// 	reinterpret_cast<const unsigned char *>(get_version().substr(get_version().size() - 32, 32).data()));

	std::array<unsigned char, crypto_box_SECRETKEYBYTES> sealedBoxSecretKey{};
	if (crypto_pwhash(sealedBoxSecretKey.data(), crypto_box_SECRETKEYBYTES, password.data(), password.size(),
					  salt.data(), crypto_pwhash_OPSLIMIT_SENSITIVE, crypto_pwhash_MEMLIMIT_SENSITIVE,
					  crypto_pwhash_ALG_DEFAULT) != 0)
	{
		spdlog::error("Can't hash the input key for sealed box");
		return EXIT_FAILURE;
	}

	std::array<unsigned char, crypto_box_PUBLICKEYBYTES> sealedBoxPublicKey{};
	if (crypto_scalarmult_base(sealedBoxPublicKey.data(), sealedBoxSecretKey.data()) != 0)
	{
		spdlog::error("Can't generate public key for sealed box");
		return EXIT_FAILURE;
	}

	// Encrypt certificate to sealed box
	std::array<unsigned char, crypto_box_SEALBYTES + 40> cipheredData{};
	if (crypto_box_seal(cipheredData.data(), reinterpret_cast<unsigned char *>(z85_sk.data()), 40,
						sealedBoxPublicKey.data()) != 0)
	{
		spdlog::error("Can't encrypt the ZeroMQ secret");
		return EXIT_FAILURE;
	}

	// Dump data to files
	std::ofstream zmqSecretFile(outputPath + "/zmq.enc", std::ios_base::binary);
	std::ofstream zmqPublicFile(outputPath + "/zmq.pub", std::ios_base::binary);

	if (!zmqSecretFile.is_open() || !zmqPublicFile.is_open())
	{
		spdlog::error("Can't open output files");
		return EXIT_FAILURE;
	}

	if (zmqSecretFile.write(reinterpret_cast<const char *>(cipheredData.data()), cipheredData.size()).fail())
	{
		spdlog::error("Can't write data to encrypted file");
		return EXIT_FAILURE;
	}

	if (zmqPublicFile.write(z85_pk.data(), z85_pk.size()).fail())
	{
		spdlog::error("Can't write data to public key file");
		return EXIT_FAILURE;
	}

	if (input.cmdOptionExists("--writePlainSecretKey"))
	{
		std::ofstream zmqPlainSecretFile(outputPath + "/zmq.key", std::ios_base::binary);
		if (!zmqPlainSecretFile.is_open())
		{
			spdlog::error("Can't open output file");
			return EXIT_FAILURE;
		}

		if (zmqPlainSecretFile.write(z85_sk.data(), z85_sk.size()).fail())
		{
			spdlog::error("Can't write data to secret key file");
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
