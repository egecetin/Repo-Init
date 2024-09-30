#include "utils/BaseServerStats.hpp"

#include <prometheus/counter.h>
#include <prometheus/summary.h>

void BaseServerStats::initBaseStats(const std::shared_ptr<prometheus::Registry> &reg, const std::string name)
{
	// Command stats
	_succeededCommand = &prometheus::BuildCounter()
							 .Name(name + "succeeded_commands")
							 .Help("Number of succeeded commands")
							 .Register(*reg)
							 .Add({});
	_failedCommand = &prometheus::BuildCounter()
						  .Name(name + "failed_commands")
						  .Help("Number of failed commands")
						  .Register(*reg)
						  .Add({});
	_totalCommand = &prometheus::BuildCounter()
						 .Name(name + "received_commands")
						 .Help("Number of received commands")
						 .Register(*reg)
						 .Add({});

	// Performance stats
	_processingTime = &prometheus::BuildSummary()
						   .Name(name + "processing_time")
						   .Help("Command processing performance")
						   .Register(*reg)
						   .Add({}, QUANTILE_DEFAULTS);
	_maxProcessingTime = &prometheus::BuildGauge()
							  .Name(name + "maximum_processing_time")
							  .Help("Maximum value of the command processing performance")
							  .Register(*reg)
							  .Add({});
	_minProcessingTime = &prometheus::BuildGauge()
							  .Name(name + "minimum_processing_time")
							  .Help("Minimum value of the command processing performance")
							  .Register(*reg)
							  .Add({});
}
