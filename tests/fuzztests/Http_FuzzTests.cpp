#include "Utils.hpp"
#include "connection/Http.hpp"
#include "test-static-definitions.h"

#include <algorithm>
#include <iostream>
#include <thread>

static uint64_t counter = 0;

#define ECHO_SERVER_PORT 9000

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
	int nRuns = std::stoi(findOptionValue("runs", *argc, *argv));

	// Launch echo server so we can fuzz both send and receive functions at the same time
	std::thread th([nRuns]() {
		return system(("python3 " + std::string(TEST_REST_ECHO_SERVER_PY_PATH) + " --port=" +
					   std::to_string(ECHO_SERVER_PORT) + " --count=" + std::to_string(nRuns) + " >/dev/null")
						  .c_str());
	});
	th.detach();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	static HTTP httpServer("http://localhost:" + std::to_string(ECHO_SERVER_PORT));

	HttpStatus::Code sendCode;
	std::string recvBuffer, payload(reinterpret_cast<const char *>(data), size);
	switch (counter % 2)
	{
	case 0: // POST
		if (httpServer.sendPOSTRequest("/test", payload, recvBuffer, sendCode) == CURLE_COULDNT_CONNECT)
			__builtin_trap();
		break;
	case 1: // PUT
		if (httpServer.sendPUTRequest("/test", payload, recvBuffer, sendCode) == CURLE_COULDNT_CONNECT)
			__builtin_trap();
		break;
	default:
		return -1;
	}
	++counter;

	return 0;
}
