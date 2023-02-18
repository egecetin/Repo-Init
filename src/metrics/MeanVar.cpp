#include "metrics/MeanVar.hpp"

#include <float.h>

void MeanVarTracker::updateStatistic(double newValue)
{
	eventCtr->Increment();
	qMeasurements.push(newValue);

	double counter = eventCtr->Value();
	double oldMean = meanVal->Value();
	double movOldMean = movingMeanVal->Value();

	// Knuth algorithm for global stats
	meanVal->Set(oldMean + (newValue - oldMean) / counter);
	stdBuffVal = stdBuffVal + (newValue - oldMean) * (newValue - meanVal->Value());
	if (counter > 1)
		varVal->Set(stdBuffVal / (counter - 1));

	// Knuth algorithm for moving stats
	if (counter > winLenInternal)
	{
		movingMeanVal->Set(movOldMean +
						   ((newValue - movOldMean) - (qMeasurements.front() - movOldMean)) / winLenInternal);
		movStdBuffVal = movStdBuffVal + (newValue - movOldMean) * (newValue - movingMeanVal->Value()) -
						(qMeasurements.front() - movOldMean) * (qMeasurements.front() - movingMeanVal->Value());

		if (counter > 1)
			movingVarVal->Set(movStdBuffVal / (counter - 1));
	}
	else
	{
		movingMeanVal->Set(meanVal->Value());
		movingVarVal->Set(varVal->Value());
	}

	// Min - max values
	if (newValue > maxVal->Value())
		maxVal->Set(newValue);
	if (newValue < minVal->Value())
		minVal->Set(newValue);

	if (qMeasurements.size() > winLenInternal)
		qMeasurements.pop();
}

MeanVarTracker::MeanVarTracker(std::shared_ptr<prometheus::Registry> &reg, const std::string &name, const size_t winLen,
							   const uint64_t id)
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

void MeanVarTracker::updateValue(double val) { updateStatistic(val); }
