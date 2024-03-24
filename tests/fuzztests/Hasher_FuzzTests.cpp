#include "utils/Hasher.hpp"

#include <cstring>
#include <string>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	char seederBuffer[4096] = {'\0'};

	std::string dataStr(reinterpret_cast<const char *>(data), size);
	constHasher(dataStr.c_str());

	memcpy(seederBuffer, data, size > 4096 ? 4096 : size);
	constSeeder(seederBuffer);

	return 0;
}
