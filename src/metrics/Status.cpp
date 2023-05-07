#include "metrics/Status.hpp"

#include <prometheus/counter.h>
#include <prometheus/gauge.h>

StatusTracker::StatusTracker(const std::shared_ptr<prometheus::Registry> &reg, const std::string &name,
							 uint64_t metricID)
{
	// Register values
	activeCtr = &prometheus::BuildGauge()
					 .Name(name + "_active_event_ctr_" + std::to_string(metricID))
					 .Help("Currently active number of events of " + name)
					 .Register(*reg)
					 .Add({});
	totalCtr = &prometheus::BuildCounter()
					.Name(name + "_total_event_ctr_" + std::to_string(metricID))
					.Help("Total occurrences of " + name)
					.Register(*reg)
					.Add({});
	successCtr = &prometheus::BuildCounter()
					  .Name(name + "_success_event_ctr_" + std::to_string(metricID))
					  .Help("Successful events of " + name)
					  .Register(*reg)
					  .Add({});
	failedCtr = &prometheus::BuildCounter()
					 .Name(name + "_fail_event_ctr_" + std::to_string(metricID))
					 .Help("Failed events of " + name)
					 .Register(*reg)
					 .Add({});
}

void StatusTracker::incrementActive() { activeCtr->Increment(); }

void StatusTracker::incrementSuccess()
{
	successCtr->Increment();
	totalCtr->Increment();
	if (activeCtr->Value() > 0)
	{
		activeCtr->Decrement();
	}
}

void StatusTracker::incrementFail()
{
	failedCtr->Increment();
	totalCtr->Increment();
	if (activeCtr->Value() > 0)
	{
		activeCtr->Decrement();
	}
}
