#include "metrics/Performance.hpp"

#include <immintrin.h>
#include <x86intrin.h>

PerformanceTracker::PerformanceTracker(std::shared_ptr<prometheus::Registry> &reg, const std::string &name,
									   const uint64_t tscHz, const size_t winLen, const uint64_t id)
	: MeanVarTracker(reg, name + "_timing", winLen, id)
{
	tscHzInternal = tscHz;
	lastTimeCtr = 0;
}

void PerformanceTracker::startTimer()
{
	_mm_lfence();
	lastTimeCtr = __rdtsc();
	_mm_lfence();
}

double PerformanceTracker::endTimer()
{
	_mm_lfence();
	uint64_t currCtr = __rdtsc();
	_mm_lfence();

	double val = double(currCtr - lastTimeCtr) / tscHzInternal;
	updateStatistic(val);
	return val;
}
