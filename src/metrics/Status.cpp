#include "metrics/Status.hpp"

#include <prometheus/counter.h>
#include <prometheus/gauge.h>

StatusTracker::StatusTracker(std::shared_ptr<prometheus::Registry> reg, const std::string &name, const uint64_t id)
{
	// Register values
	activeCtr = &prometheus::BuildGauge()
					 .Name(name + "_active_event_ctr_" + std::to_string(id))
					 .Help("Currently active number of events of " + name)
					 .Register(*reg)
					 .Add({});
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
