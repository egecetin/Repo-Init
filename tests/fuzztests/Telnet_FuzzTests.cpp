#include "TelnetClient.hpp"
#include "telnet/TelnetServer.hpp"

#include <spdlog/spdlog.h>

#include <iostream>
#include <string>
#include <thread>

#define TELNET_SERVER_PORT 9001

class TelnetWrapper {
  private:
	std::shared_ptr<TelnetServer> server;

  public:
	explicit TelnetWrapper(uint16_t port)
	{
		server = std::make_shared<TelnetServer>();
		if (!server || !(server->initialise(port, nullptr)))
		{
			throw std::runtime_error("Can't init telnet");
		}

		server->connectedCallback(TelnetConnectedCallback);
		server->newLineCallback(TelnetMessageCallback);
		server->tabCallback(TelnetTabCallback);
	}
};

extern "C" int LLVMFuzzerInitialize(int *, char ***)
{
	spdlog::set_level(spdlog::level::off);
	return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	static TelnetWrapper server(TELNET_SERVER_PORT);
	static TelnetClient client("127.0.0.1", TELNET_SERVER_PORT);

	const auto command = std::string(reinterpret_cast<const char *>(data), size);
	return client.sendCommand(command);
}
