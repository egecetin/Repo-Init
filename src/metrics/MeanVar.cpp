#include "metrics/MeanVar.hpp"

#include <float.h>

MeanVarTracker::MeanVarTracker(std::shared_ptr<prometheus::Registry> &reg, const std::string &name,
							   const size_t winLen, const uint64_t id)
{
    metricName = name;
	trackerID = id;
	winLenInternal = winLen;

	// Set all stats
	stdBuffVal = 0.0;
	movStdBuffVal = 0.0;

	// Register values
	eventCtr = &prometheus::BuildCounter()
					.Name(name + "_event_ctr_" + std::to_string(id))
					.Help("Number of occurrences of " + name)
					.Register(*reg)
					.Add({});
	meanVal = &prometheus::BuildGauge()
					  .Name(name + "_mean_" + std::to_string(id))
					  .Help("Mean of " + name)
					  .Register(*reg)
					  .Add({});
	varVal = &prometheus::BuildGauge()
					 .Name(name + "_var_" + std::to_string(id))
					 .Help("Variance of " + name)
					 .Register(*reg)
					 .Add({});
	movingMeanVal = &prometheus::BuildGauge()
							.Name(name + "_moving_mean_" + std::to_string(id))
							.Help("Moving mean of " + name)
							.Register(*reg)
							.Add({});
	movingVarVal = &prometheus::BuildGauge()
						   .Name(name + "_moving_var_" + std::to_string(id))
						   .Help("Moving variance of " + name)
						   .Register(*reg)
						   .Add({});
	maxVal = &prometheus::BuildGauge()
					 .Name(name + "_max_" + std::to_string(id))
					 .Help("Maximum " + name)
					 .Register(*reg)
					 .Add({});
	minVal = &prometheus::BuildGauge()
					 .Name(name + "_min_" + std::to_string(id))
					 .Help("Minimum " + name)
					 .Register(*reg)
					 .Add({});
	windowLength = &prometheus::BuildInfo()
						.Name("win_len")
						.Help("Window length of moving statistical values")
						.Register(*reg)
						.Add({{"init_time", std::to_string(winLenInternal)}});

	// Set initial values
	meanVal->Set(0.0);
	varVal->Set(0.0);
	movingMeanVal->Set(0.0);
	movingVarVal->Set(0.0);
	maxVal->Set(0.0);
	minVal->Set(DBL_MAX);
}