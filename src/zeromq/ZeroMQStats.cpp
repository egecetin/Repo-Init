#include "zeromq/ZeroMQStats.hpp"

#include <date/date.h>
#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <prometheus/info.h>
#include <prometheus/summary.h>

ZeroMQStats::ZeroMQStats(const std::shared_ptr<prometheus::Registry> &reg,
					const std::string &prependName)
{
	if (!reg)
	{
		throw std::invalid_argument("Can't init ZeroMQ statistics. Registry is null");
	}

	const auto name = prependName.empty() ? "zeromq_" : prependName + "_zeromq_";

	// Stats from base class
	initBaseStats(reg, name);

	// Basic information
	_infoFamily = &prometheus::BuildInfo().Name("zeromq").Help("ZeroMQ server information").Register(*reg);

	_infoFamily->Add({{"init_time", date::format("%FT%TZ", date::floor<std::chrono::nanoseconds>(
															   std::chrono::high_resolution_clock::now()))}});
	_infoFamily->Add({{"performance_unit", "nanoseconds"}});

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
	_succeededCommandParts = &prometheus::BuildCounter()
								  .Name(name + "succeeded_command_parts")
								  .Help("Number of received succeeded message parts")
								  .Register(*reg)
								  .Add({});
	_failedCommandParts = &prometheus::BuildCounter()
							   .Name(name + "failed_command_parts")
							   .Help("Number of received failed message parts")
							   .Register(*reg)
							   .Add({});
	_totalCommandParts = &prometheus::BuildCounter()
							  .Name(name + "total_command_parts")
							  .Help("Number of received total message parts")
							  .Register(*reg)
							  .Add({});

	// Bandwidth stats
	_totalUploadBytes =
		&prometheus::BuildCounter().Name(name + "uploaded_bytes").Help("Total uploaded bytes").Register(*reg).Add({});
	_totalDownloadBytes = &prometheus::BuildCounter()
							   .Name(name + "downloaded_bytes")
							   .Help("Total downloaded bytes")
							   .Register(*reg)
							   .Add({});

	// Set defaults
	_minProcessingTime->Set(std::numeric_limits<double>::max());
}

void ZeroMQStats::consumeStats(const std::vector<zmq::message_t> &recvMsgs, const std::vector<zmq::message_t> &sendMsgs,
							   const ZeroMQServerStats &serverStats)
{
	_succeededCommand->Increment(static_cast<double>(serverStats.isSuccessful));
	_failedCommand->Increment(static_cast<double>(!serverStats.isSuccessful));
	_totalCommand->Increment();

	for (const auto &entry : recvMsgs)
	{
		_totalCommandParts->Increment();
		_totalDownloadBytes->Increment(static_cast<double>(entry.size()));

		if (serverStats.isSuccessful)
		{
			_succeededCommandParts->Increment();
		}
		else
		{
			_failedCommandParts->Increment();
		}
	}

	for (const auto &entry : sendMsgs)
	{
		_totalUploadBytes->Increment(static_cast<double>(entry.size()));
	}

	const auto processTime =
		static_cast<double>((serverStats.processingTimeEnd - serverStats.processingTimeStart).count());
	_processingTime->Observe(processTime);
	if (processTime < _minProcessingTime->Value())
	{
		_minProcessingTime->Set(processTime);
	}
	if (processTime > _maxProcessingTime->Value())
	{
		_maxProcessingTime->Set(processTime);
	}
}
