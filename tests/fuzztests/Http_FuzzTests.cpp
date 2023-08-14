#include "Utils.hpp"
#include "connection/Http.hpp"
#include "test-static-definitions.h"

#include <algorithm>
#include <iostream>
#include <thread>

#define ECHO_SERVER_PORT 9000

class PythonScriptWrapper {
  private:
	FILE *fPtr = nullptr;

  public:
	void start(const std::string &command)
	{
		fPtr = popen(command.c_str(), "r");
		if (fPtr == nullptr)
		{
			throw std::runtime_error("Can't open pipe!");
		}
	}

	~PythonScriptWrapper()
	{
		if (fPtr)
		{
			pclose(fPtr);
		}
	}
};

static uint64_t counter = 0;
static PythonScriptWrapper pythonScript;

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
	int nRuns = std::stoi(findOptionValue("runs", *argc, *argv));

	// Launch echo server so we can fuzz both send and receive functions at the same time
	pythonScript.start(("python3 " + std::string(TEST_REST_ECHO_SERVER_PY_PATH) + " --port=" +
						std::to_string(ECHO_SERVER_PORT) + " --count=" + std::to_string(nRuns) + " >/dev/null"));
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	static HTTP httpServer("http://localhost:" + std::to_string(ECHO_SERVER_PORT));

	HttpStatus::Code sendCode;
	std::string recvBuffer, dataStr(reinterpret_cast<const char *>(data), size);
	switch (counter % 2)
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
	case 3: // GET
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
