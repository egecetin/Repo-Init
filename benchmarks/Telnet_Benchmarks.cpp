#include "TelnetClient.hpp"
#include "telnet/TelnetServer.hpp"

#include <benchmark/benchmark.h>

#define TELNET_SERVER_PORT 10001

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

static void Telnet_Benchmark(benchmark::State &state)
{
	static TelnetWrapper server(TELNET_SERVER_PORT);
	static TelnetClient client("127.0.0.1", TELNET_SERVER_PORT);

	const auto command = "ping";
	for (auto _ : state)
	{
		if (!client.sendCommand(command))
		{
			state.SkipWithError("Can't send Telnet command to server");
			return;
		}
	}
}
BENCHMARK(Telnet_Benchmark);
