#include "utils/Hasher.hpp"

#include <benchmark/benchmark.h>

static void Benchmark_Hasher(benchmark::State &state)
{
	std::string testString = "Lorem ipsum dolor sit amet";
	for (auto _ : state)
	{
		benchmark::DoNotOptimize(constHasher(testString.c_str()));
	}
}
BENCHMARK(Benchmark_Hasher);

static void Benchmark_Seeder(benchmark::State &state)
{
	char testString[] = "Lorem ipsum dolor sit amet";
	for (auto _ : state)
	{
		benchmark::DoNotOptimize(constSeeder(testString));
	}
}
BENCHMARK(Benchmark_Seeder);
