#include "test-static-definitions.h"

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#ifdef XXX_ENABLE_MEMLEAK_CHECK
#include <memplumber.h>
#endif

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	int retval = EXIT_FAILURE;

	spdlog::set_level(spdlog::level::off);

#ifdef XXX_ENABLE_MEMLEAK_CHECK
	MemPlumber::start();
#endif
	retval = RUN_ALL_TESTS();
	spdlog::shutdown();
#ifdef XXX_ENABLE_MEMLEAK_CHECK
	MemPlumber::stop();

	// Check memory leak
	size_t memLeakCtr;
	uint64_t memLeakSz;
	MemPlumber::memLeakCheck(memLeakCtr, memLeakSz, true);

	// Comes from gtest itself
	if (memLeakCtr == 1 && memLeakSz == 8)
		printf("No memory leak detected!\n");
	else if (memLeakCtr || memLeakSz)
	{
		printf("Number of leaked objects: %d\nTotal amount of memory leaked: %d[bytes]\n", (int)memLeakCtr,
			   (int)memLeakSz);
		retval = EXIT_FAILURE;
	}
#endif

	return retval;
}
