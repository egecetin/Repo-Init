#include "Performance.hpp"
#include "Utils.hpp"

#include <stdexcept>

#include <cpuid.h>
#include <float.h>
#include <immintrin.h>
#include <x86intrin.h>

#include <spdlog/spdlog.h>

Reporter *mainPrometheusHandler;

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

StatusTracker::StatusTracker(std::shared_ptr<prometheus::Registry> &reg, const std::string &name, const uint64_t id)
{
	metricName = name;
	trackerID = id;

	// Register values
	totalCtr = &prometheus::BuildCounter()
					.Name(name + "_total_event_ctr_" + std::to_string(id))
					.Help("Total occurrences of " + name)
					.Register(*reg)
					.Add({});
	successCtr = &prometheus::BuildCounter()
					  .Name(name + "_success_event_ctr_" + std::to_string(id))
					  .Help("Successful events of " + name)
					  .Register(*reg)
					  .Add({});
	failedCtr = &prometheus::BuildCounter()
					 .Name(name + "_fail_event_ctr_" + std::to_string(id))
					 .Help("Failed events of " + name)
					 .Register(*reg)
					 .Add({});
	activeCtr = &prometheus::BuildGauge()
					 .Name(name + "_active_event_ctr_" + std::to_string(id))
					 .Help("Currently active number of events of " + name)
					 .Register(*reg)
					 .Add({});
}

void StatusTracker::incrementActive() { activeCtr->Increment(); }

void StatusTracker::incrementSuccess()
{
	successCtr->Increment();
	totalCtr->Increment();
	if (activeCtr->Value() > 0)
		activeCtr->Decrement();
}

void StatusTracker::incrementFail()
{
	failedCtr->Increment();
	totalCtr->Increment();
	if (activeCtr->Value() > 0)
		activeCtr->Decrement();
}

Reporter::Reporter(const std::string &serverAddr)
{
	// From linux kernel (https://github.com/torvalds/linux/blob/master/tools/power/x86/turbostat/turbostat.c)
	uint32_t eax_crystal, ebx_tsc, crystal_hz, edx;
	eax_crystal = ebx_tsc = crystal_hz = edx = 0;
	__cpuid(0x15, eax_crystal, ebx_tsc, crystal_hz, edx);

	if (!ebx_tsc || !crystal_hz || !eax_crystal)
	{
		spdlog::error("Can't determine TSC frequency");
		tsc_hz = 1;
	}
	else
		tsc_hz = (uint64_t)crystal_hz * ebx_tsc / eax_crystal;

	// Init service
	mainExposer = std::make_unique<prometheus::Exposer>(serverAddr);
	spdlog::debug("Prometheus server start at {}", serverAddr);

	struct timespec ts;
	clock_gettime(CLOCK_TAI, &ts);

	auto reg = std::make_shared<prometheus::Registry>();
	initTime = &prometheus::BuildInfo()
					.Name("start_time")
					.Help("Initialization time of the application")
					.Register(*reg)
					.Add({{"init_time", std::to_string(ts.tv_sec)}});
	vRegister.push_back(reg);

	mainExposer->RegisterCollectable(reg);
}

std::shared_ptr<PerformanceTracker> Reporter::addNewPerfTracker(const std::string &name, uint64_t id)
{
	std::lock_guard<std::mutex> guard(guardLock);

	auto reg = std::make_shared<prometheus::Registry>();
	auto tracker = std::make_shared<PerformanceTracker>(reg, name, tsc_hz, id);
	mainExposer->RegisterCollectable(reg);

	vRegister.push_back(reg);
	vPerfTracker.push_back(tracker);

	return tracker;
}

std::shared_ptr<StatusTracker> Reporter::addNewStatTracker(const std::string &name, uint64_t id)
{
	std::lock_guard<std::mutex> guard(guardLock);

	auto reg = std::make_shared<prometheus::Registry>();
	auto tracker = std::make_shared<StatusTracker>(reg, name, id);
	mainExposer->RegisterCollectable(reg);

	vRegister.push_back(reg);
	vStatTracker.push_back(tracker);

	return tracker;
}
