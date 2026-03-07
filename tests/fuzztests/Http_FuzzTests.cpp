#include "connection/Http.hpp"

#include "EchoServer.hpp"
#include "test-static-definitions.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <iostream>
#include <thread>

#define ECHO_SERVER_PORT 9000

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
	spdlog::set_level(spdlog::level::off);
	return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	static uint64_t counter;
	static EchoServer echoServer(ECHO_SERVER_PORT);
	static HTTP httpServer("http://localhost:" + std::to_string(ECHO_SERVER_PORT));

	HttpStatus::Code sendCode;
	std::string recvBuffer, dataStr(reinterpret_cast<const char *>(data), size);
	switch (counter % 4)
	{
	case 0: // POST
		if (httpServer.sendPOSTRequest("/test", dataStr, recvBuffer, sendCode) == CURLE_COULDNT_CONNECT)
		{
			__builtin_trap();
		}
		break;
	case 1: // PUT
		if (httpServer.sendPUTRequest("/test", dataStr, recvBuffer, sendCode) == CURLE_COULDNT_CONNECT)
		{
			__builtin_trap();
		}
		break;
	case 2: // GET
		if (httpServer.sendGETRequest(dataStr, recvBuffer, sendCode) == CURLE_COULDNT_CONNECT)
		{
			__builtin_trap();
		}
		break;
	case 3: // HEAD
		if (httpServer.sendHEADRequest(dataStr, recvBuffer, sendCode) == CURLE_COULDNT_CONNECT)
		{
			__builtin_trap();
		}
		break;
	default:
		return -1;
	}
	++counter;

	return 0;
}
