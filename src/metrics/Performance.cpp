#include "metrics/Performance.hpp"

#include <float.h>
#include <immintrin.h>
#include <x86intrin.h>

void PerformanceTracker::updateStatistic(double newValue)
{
	eventCtr->Increment();
	qMeasurements.push(newValue);

	double counter = eventCtr->Value();
	double oldMean = meanTiming->Value();
	double movOldMean = movingMeanTiming->Value();

	// Knuth algorithm for global stats
	meanTiming->Set(oldMean + (newValue - oldMean) / counter);
	stdBuffTiming = stdBuffTiming + (newValue - oldMean) * (newValue - meanTiming->Value());
	if (counter > 1)
		varTiming->Set(stdBuffTiming / (counter - 1));

	// Knuth algorithm for moving stats
	if (counter > winLenInternal)
	{
		movingMeanTiming->Set(movOldMean +
							  ((newValue - movOldMean) - (qMeasurements.front() - movOldMean)) / winLenInternal);
		movStdBuffTiming = movStdBuffTiming + (newValue - movOldMean) * (newValue - movingMeanTiming->Value()) -
						   (qMeasurements.front() - movOldMean) * (qMeasurements.front() - movingMeanTiming->Value());

		if (counter > 1)
			movingVarTiming->Set(movStdBuffTiming / (counter - 1));
	}
	else
	{
		movingMeanTiming->Set(meanTiming->Value());
		movingVarTiming->Set(varTiming->Value());
	}

	// Min - max values
	if (newValue > maxTiming->Value())
		maxTiming->Set(newValue);
	if (newValue < minTiming->Value())
		minTiming->Set(newValue);

	if (qMeasurements.size() > winLenInternal)
		qMeasurements.pop();
}

PerformanceTracker::PerformanceTracker(std::shared_ptr<prometheus::Registry> &reg, const std::string &name,
									   const uint64_t tscHz, const size_t winLen, const uint64_t id)
{
	metricName = name;
	trackerID = id;
	tscHzInternal = tscHz;
	winLenInternal = winLen;

	// Set all stats
	lastTimeCtr = 0;
	stdBuffTiming = 0.0;
	movStdBuffTiming = 0.0;

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
	movingMeanTiming = &prometheus::BuildGauge()
							.Name(name + "_moving_mean_timing_" + std::to_string(id))
							.Help("Moving mean of processing time")
							.Register(*reg)
							.Add({});
	movingVarTiming = &prometheus::BuildGauge()
						   .Name(name + "_moving_var_timing_" + std::to_string(id))
						   .Help("Moving variance of processing time")
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
	windowLength = &prometheus::BuildInfo()
						.Name("win_len")
						.Help("Window length of moving statistical values")
						.Register(*reg)
						.Add({{"init_time", std::to_string(winLenInternal)}});

	// Set initial values
	meanTiming->Set(0.0);
	varTiming->Set(0.0);
	movingMeanTiming->Set(0.0);
	movingVarTiming->Set(0.0);
	maxTiming->Set(0.0);
	minTiming->Set(DBL_MAX);
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
