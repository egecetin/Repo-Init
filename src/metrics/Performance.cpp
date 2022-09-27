#include "metrics/Performance.hpp"

#include <float.h>
#include <immintrin.h>
#include <x86intrin.h>

void PerformanceTracker::updateStatistic(double newValue)
{
	eventCtr->Increment();
	double oldMean = meanTiming->Value();

	// Knuth algorithm
	meanTiming->Set(meanTiming->Value() + (newValue - meanTiming->Value()) / eventCtr->Value());
	stdBuffTiming = stdBuffTiming + (newValue - oldMean) * (newValue - meanTiming->Value());
	if (eventCtr->Value() > 1)
		varTiming->Set(stdBuffTiming / (eventCtr->Value() - 1));

	if (newValue > maxTiming->Value())
		maxTiming->Set(newValue);
	if (newValue < minTiming->Value())
		minTiming->Set(newValue);
}

PerformanceTracker::PerformanceTracker(std::shared_ptr<prometheus::Registry> &reg, const std::string &name,
									   const uint64_t tsc_hz, const uint64_t id)
{
	metricName = name;
	trackerID = id;
	tsc_hz_internal = tsc_hz;

	// Set all stats
	lastTimeCtr = 0;
	stdBuffTiming = 0.0;

	// Register values
	eventCtr = &prometheus::BuildCounter()
					.Name(name + "_event_ctr_" + std::to_string(id))
					.Help("Number of occurrences of " + name)
					.Register(*reg)
					.Add({});
	meanTiming = &prometheus::BuildGauge()
					  .Name(name + "_mean_timing_" + std::to_string(id))
					  .Help("Mean of processing time")
					  .Register(*reg)
					  .Add({});
	varTiming = &prometheus::BuildGauge()
					 .Name(name + "_var_timing_" + std::to_string(id))
					 .Help("Variance of processing time")
					 .Register(*reg)
					 .Add({});
	maxTiming = &prometheus::BuildGauge()
					 .Name(name + "_max_timing_" + std::to_string(id))
					 .Help("Maximum processing time")
					 .Register(*reg)
					 .Add({});
	minTiming = &prometheus::BuildGauge()
					 .Name(name + "_min_timing_" + std::to_string(id))
					 .Help("Minimum processing time")
					 .Register(*reg)
					 .Add({});

	// Set initial values
	meanTiming->Set(0.0);
	varTiming->Set(0.0);
	maxTiming->Set(0.0);
	minTiming->Set(DBL_MAX);
}

void PerformanceTracker::startTimer()
{
	_mm_lfence();
	lastTimeCtr = __rdtsc();
	_mm_lfence();
}

void PerformanceTracker::endTimer()
{
	_mm_lfence();
	uint64_t currCtr = __rdtsc();
	_mm_lfence();

	updateStatistic(double(currCtr - lastTimeCtr) / tsc_hz_internal);
}
