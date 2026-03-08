#include "utils/ErrorHelpers.hpp"

#include <benchmark/benchmark.h>
#include <spdlog/spdlog.h>

int main(int argc, char **argv)
{
	spdlog::set_level(spdlog::level::off);

	// Internal variables
	vCheckFlag.emplace_back("Test Disabled Info", std::make_unique<std::atomic_flag>(false));
	vCheckFlag.emplace_back("Test Enabled Info", std::make_unique<std::atomic_flag>(true));

	::benchmark::Initialize(&argc, argv);
	::benchmark::RunSpecifiedBenchmarks();

	return EXIT_SUCCESS;
}