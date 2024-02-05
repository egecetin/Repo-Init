#include "metrics/Performance.hpp"

#include <prometheus/gauge.h>
#include <prometheus/summary.h>

#define QUANTILE_DEFAULTS                                                                                              \
	prometheus::Summary::Quantiles                                                                                     \
	{                                                                                                                  \
		{0.5, 0.1}, {0.9, 0.1}, { 0.99, 0.1 }                                                                          \
	}

PerformanceTracker::PerformanceTracker(const std::shared_ptr<prometheus::Registry> &reg, const std::string &name,
									   uint64_t metricID)
{
	perfTiming = &prometheus::BuildSummary()
					  .Name(name + "_processing_time_" + std::to_string(metricID))
					  .Help(name + " processing performance")
					  .Register(*reg)
					  .Add({}, QUANTILE_DEFAULTS);
	maxTiming = &prometheus::BuildGauge()
					 .Name(name + "_maximum_processing_time_" + std::to_string(metricID))
					 .Help("Maximum value of the " + name + " processing performance")
					 .Register(*reg)
					 .Add({});
	minTiming = &prometheus::BuildGauge()
					 .Name(name + "_minimum_processing_time_" + std::to_string(metricID))
					 .Help("Minimum value of the " + name + " processing performance")
					 .Register(*reg)
					 .Add({});

	minTiming->Set(std::numeric_limits<double>::max());
}

void PerformanceTracker::startTimer() { startTime = std::chrono::high_resolution_clock::now(); }

double PerformanceTracker::endTimer()
{
	const auto val = static_cast<double>((std::chrono::high_resolution_clock::now() - startTime).count());

	perfTiming->Observe(val);
	if (val < minTiming->Value())
	{
		minTiming->Set(val);
	}
	if (val > maxTiming->Value())
	{
		maxTiming->Set(val);
	}

	return val;
}
