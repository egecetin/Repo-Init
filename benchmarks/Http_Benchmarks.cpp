#include "connection/Http.hpp"

#include "EchoServer.hpp"

#include <benchmark/benchmark.h>

#define ECHO_SERVER_PORT 10000

static void Http_Benchmark(benchmark::State &state)
{
	uint64_t counter = 0;
	static EchoServer echoServer(ECHO_SERVER_PORT);
	static HTTP httpServer("http://localhost:" + std::to_string(ECHO_SERVER_PORT));

	HttpStatus::Code sendCode;
	std::string sendStr = "Lorem ipsum dolor sit amet", recvStr;
	for (auto _ : state)
	{
		CURLcode returnCode = CURL_LAST;
		switch (counter % 4)
		{
		case 0:
			returnCode = httpServer.sendPOSTRequest("/test", sendStr, recvStr, sendCode);
			break;
		case 1:
			returnCode = httpServer.sendPUTRequest("/test", sendStr, recvStr, sendCode);
			break;
		case 2:
			returnCode = httpServer.sendGETRequest(sendStr, recvStr, sendCode);
			break;
		case 3:
			returnCode = httpServer.sendHEADRequest(sendStr, recvStr, sendCode);
			break;
		default:
			state.SkipWithError("Counter mismatch");
			return;
		}

		if (returnCode == CURLE_COULDNT_CONNECT)
		{
			state.SkipWithError("Can't connect to server");
			return;
		}
		++counter;
	}
}
BENCHMARK(Http_Benchmark);
