#include "utils/Hasher.hpp"

#include <benchmark/benchmark.h>

static void Hasher_Benchmark(benchmark::State &state)
{
	std::string testString = "Lorem ipsum dolor sit amet";
	for (auto _ : state)
	{
		benchmark::DoNotOptimize(constHasher(testString.c_str()));
	}
}
BENCHMARK(Hasher_Benchmark);

static void Seeder_Benchmark(benchmark::State &state)
{
	char testString[] = "Lorem ipsum dolor sit amet";
	for (auto _ : state)
	{
		benchmark::DoNotOptimize(constSeeder(testString));
	}
}
BENCHMARK(Seeder_Benchmark);
