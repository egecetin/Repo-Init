#include "zeromq/ZeroMQStats.hpp"

#include <date/date.h>
#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <prometheus/info.h>
#include <prometheus/summary.h>

ZeroMQStats::ZeroMQStats(const std::shared_ptr<prometheus::Registry> &reg)
{
	if (!reg)
	{
		throw std::runtime_error("Can't init ZeroMQ statistics. Registry is null");
	}

	// Basic information
	infoFamily = &prometheus::BuildInfo().Name("zeromq").Help("ZeroMQ server information").Register(*reg);

	infoFamily->Add({{"init_time", date::format("%FT%TZ", date::floor<std::chrono::nanoseconds>(
															  std::chrono::high_resolution_clock::now()))}});
	infoFamily->Add({{"performance_unit", "nanoseconds"}});

	// Performance stats
	processingTime = &prometheus::BuildSummary()
						  .Name("zeromq_processing_time")
						  .Help("Command processing performance")
						  .Register(*reg)
						  .Add({}, prometheus::Summary::Quantiles{{0.5, 0.1}, {0.9, 0.1}, {0.99, 0.1}});
	maxProcessingTime = &prometheus::BuildGauge()
							 .Name("zeromq_maximum_processing_time")
							 .Help("Maximum value of the command processing performance")
							 .Register(*reg)
							 .Add({});
	minProcessingTime = &prometheus::BuildGauge()
							 .Name("zeromq_minimum_processing_time")
							 .Help("Minimum value of the command processing performance")
							 .Register(*reg)
							 .Add({});

	// Command stats
	succeededCommand = &prometheus::BuildCounter()
							.Name("zeromq_succeeded_commands")
							.Help("Number of succeeded commands")
							.Register(*reg)
							.Add({});
	failedCommand = &prometheus::BuildCounter()
						 .Name("zeromq_failed_commands")
						 .Help("Number of failed commands")
						 .Register(*reg)
						 .Add({});
	totalCommand = &prometheus::BuildCounter()
						.Name("zeromq_received_commands")
						.Help("Number of received commands")
						.Register(*reg)
						.Add({});
	succeededCommandParts = &prometheus::BuildCounter()
								 .Name("zeromq_succeeded_command_parts")
								 .Help("Number of received succeeded message parts")
								 .Register(*reg)
								 .Add({});
	failedCommandParts = &prometheus::BuildCounter()
							  .Name("zeromq_failed_command_parts")
							  .Help("Number of received failed message parts")
							  .Register(*reg)
							  .Add({});
	totalCommandParts = &prometheus::BuildCounter()
							 .Name("zeromq_total_command_parts")
							 .Help("Number of received total message parts")
							 .Register(*reg)
							 .Add({});

	// Bandwidth stats
	totalUploadBytes =
		&prometheus::BuildCounter().Name("zeromq_uploaded_bytes").Help("Total uploaded bytes").Register(*reg).Add({});
	totalDownloadBytes = &prometheus::BuildCounter()
							  .Name("zeromq_downloaded_bytes")
							  .Help("Total downloaded bytes")
							  .Register(*reg)
							  .Add({});

	// Set defaults
	minProcessingTime->Set(std::numeric_limits<double>::max());
}

void ZeroMQStats::consumeStats(const std::vector<zmq::message_t> &recvMsgs,
							   const std::vector<zmq::message_t> &sendMsgs, const ZeroMQServerStats &serverStats)
{
	succeededCommand->Increment(static_cast<double>(serverStats.isSuccessful));
	failedCommand->Increment(static_cast<double>(!serverStats.isSuccessful));
	totalCommand->Increment();

	for (const auto &entry : recvMsgs)
	{
		totalCommandParts->Increment();
		totalDownloadBytes->Increment(static_cast<double>(entry.size()));

		if (serverStats.isSuccessful)
		{
			succeededCommandParts->Increment();
		}
		else
		{
			failedCommandParts->Increment();
		}
	}

	for (const auto &entry : sendMsgs)
	{
		totalUploadBytes->Increment(static_cast<double>(entry.size()));
	}

	const auto processTime =
		static_cast<double>((serverStats.processingTimeEnd - serverStats.processingTimeStart).count());
	processingTime->Observe(processTime);
	if (processTime < minProcessingTime->Value())
	{
		minProcessingTime->Set(processTime);
	}
	if (processTime > maxProcessingTime->Value())
	{
		maxProcessingTime->Set(processTime);
	}
}
