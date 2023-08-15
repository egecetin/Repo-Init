#include "telnet/TelnetServer.hpp"

#include <spdlog/spdlog.h>

#include <iostream>
#include <thread>

#define TELNET_SERVER_PORT 9001

// Parse fuzzer arguments
std::string findOptionValue(const std::string &option, int argc, char **argv)
{
	for (int idx = 0; idx < argc; ++idx)
	{
		const std::string argument = argv[idx];
		auto pos = argument.find("=");
		if (pos != std::string::npos)
		{
			const std::string key = argument.substr(1, pos - 1);
			if (key == option)
			{
				return argument.substr(pos + 1);
			}
		}
	}

	return "";
}

class SocketWrapper {
  private:
	int sockFd{-1};

  public:
	SocketWrapper(uint16_t port)
	{
		struct hostent *host = gethostbyname("localhost");
		if (host == nullptr)
		{
			throw std::runtime_error("Gethostbyname failed");
		}

		struct sockaddr_in server;
		server.sin_family = AF_INET;
		server.sin_port = htons(port);
		server.sin_addr.s_addr = *((unsigned long *)host->h_addr);

		sockFd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockFd < 0)
		{
			throw std::runtime_error("Socket failed");
		}

		if (connect(sockFd, (struct sockaddr *)&server, sizeof(server)) < 0)
		{
			throw std::runtime_error("Connect failed");
		}
	}

	int sendMessage(const uint8_t *data, size_t size) { return send(sockFd, data, size, 0) < 0 ? -1 : 0; }

	~SocketWrapper()
	{
		if (sockFd >= 0)
		{
			close(sockFd);
		}
	}
};

class TelnetWrapper {
  private:
	bool isActive{false};
	std::shared_ptr<TelnetServer> server;
	std::unique_ptr<std::thread> th;

	static void run(bool &isActiveFlag, std::shared_ptr<TelnetServer> serverPtr)
	{
		while (isActiveFlag)
		{
			serverPtr->update();
		}
	}

  public:
	explicit TelnetWrapper(uint16_t port)
	{
		server = std::make_shared<TelnetServer>();
		if (!server || !(server->initialise(port)))
		{
			throw std::runtime_error("Can't init telnet");
		}

		server->connectedCallback(TelnetConnectedCallback);
		server->newLineCallback(TelnetMessageCallback);
		server->tabCallback(TelnetTabCallback);

		isActive = true;
		th = std::make_unique<std::thread>(run, std::ref(isActive), server);
	}

	~TelnetWrapper()
	{
		isActive = false;
		if (th && th->joinable())
		{
			th->join();
		}
	}
};

extern "C" int LLVMFuzzerInitialize(int *, char ***)
{
	spdlog::set_level(spdlog::level::off);
	return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	static TelnetWrapper telnetWrap(TELNET_SERVER_PORT);
	static SocketWrapper socketWrap(TELNET_SERVER_PORT);

	return socketWrap.sendMessage(data, size);
}
