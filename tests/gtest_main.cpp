// #include <memplumber.h>
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	int retval = EXIT_FAILURE;

    spdlog::set_level(spdlog::level::off);

	// MemPlumber::start();
	retval = RUN_ALL_TESTS();
	// MemPlumber::stop();

	// size_t memLeakCtr;
	// uint64_t memLeakSz;
	// MemPlumber::memLeakCheck(memLeakCtr, memLeakSz, true);

	// if(memLeakCtr == 1 && memLeakSz == 8)
	//     printf("No memory leak detected! (Only os_stack_trace_getter_ from
	//     googletest-src/googletest/src/gtest.cc:6171)\n");
	// else if(memLeakCtr || memLeakSz)
	// {
	//     printf("Number of leaked objects: %d\nTotal amount of memory leaked: %d[bytes]\n", (int)memLeakCtr,
	//     (int)memLeakSz); retval = EXIT_FAILURE;
	// }

	return retval;
}
