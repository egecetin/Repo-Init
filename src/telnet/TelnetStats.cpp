#include "telnet/TelnetStats.hpp"

#include <date/date.h>
#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <prometheus/info.h>
#include <prometheus/summary.h>

#include <limits>

#define QUANTILE_DEFAULTS                                                                                              \
	prometheus::Summary::Quantiles                                                                                     \
	{                                                                                                                  \
		{0.5, 0.1}, {0.9, 0.1}, { 0.99, 0.1 }                                                                          \
	}

TelnetStats::TelnetStats(const std::shared_ptr<prometheus::Registry> &reg, uint16_t portNumber)
{
	if (!reg)
	{
		throw std::runtime_error("Can't init Telnet statistics. Registry is null");
	}

	// Basic information
	infoFamily = &prometheus::BuildInfo().Name("telnet").Help("Telnet server information").Register(*reg);

	infoFamily->Add({{"server_port", std::to_string(portNumber)}});
	infoFamily->Add({{"init_time", date::format("%FT%TZ", date::floor<std::chrono::nanoseconds>(
															  std::chrono::high_resolution_clock::now()))}});
	infoFamily->Add({{"performance_unit", "nanoseconds"}});

	// Connection stats
	activeConnection = &prometheus::BuildGauge()
							.Name("telnet_active_connections")
							.Help("Number of active connections")
							.Register(*reg)
							.Add({});
	refusedConnection = &prometheus::BuildCounter()
							 .Name("telnet_refused_connections")
							 .Help("Number of refused connections")
							 .Register(*reg)
							 .Add({});
	totalConnection = &prometheus::BuildCounter()
						   .Name("telnet_received_connections")
						   .Help("Number of received connections")
						   .Register(*reg)
						   .Add({});

	// Performance stats
	processingTime = &prometheus::BuildSummary()
						  .Name("telnet_processing_time")
						  .Help("Command processing performance")
						  .Register(*reg)
						  .Add({}, QUANTILE_DEFAULTS);
	maxProcessingTime = &prometheus::BuildGauge()
							 .Name("telnet_maximum_processing_time")
							 .Help("Maximum value of the command processing performance")
							 .Register(*reg)
							 .Add({});
	minProcessingTime = &prometheus::BuildGauge()
							 .Name("telnet_minimum_processing_time")
							 .Help("Minimum value of the command processing performance")
							 .Register(*reg)
							 .Add({});

	// Command stats
	succeededCommand = &prometheus::BuildCounter()
							.Name("telnet_succeeded_commands")
							.Help("Number of succeeded commands")
							.Register(*reg)
							.Add({});
	failedCommand = &prometheus::BuildCounter()
						 .Name("telnet_failed_commands")
						 .Help("Number of failed commands")
						 .Register(*reg)
						 .Add({});
	totalCommand = &prometheus::BuildCounter()
						.Name("telnet_received_commands")
						.Help("Number of received commands")
						.Register(*reg)
						.Add({});

	// Bandwidth stats
	totalUploadBytes =
		&prometheus::BuildCounter().Name("telnet_uploaded_bytes").Help("Total uploaded bytes").Register(*reg).Add({});
	totalDownloadBytes = &prometheus::BuildCounter()
							  .Name("telnet_downloaded_bytes")
							  .Help("Total downloaded bytes")
							  .Register(*reg)
							  .Add({});

	// Session durations
	sessionDuration = &prometheus::BuildSummary()
						   .Name("telnet_session_duration")
						   .Help("Duration of sessions")
						   .Register(*reg)
						   .Add({}, QUANTILE_DEFAULTS);
	maxSessionDuration = &prometheus::BuildGauge()
							  .Name("telnet_maximum_session_duration")
							  .Help("Maximum duration of sessions")
							  .Register(*reg)
							  .Add({});
	minSessionDuration = &prometheus::BuildGauge()
							  .Name("telnet_minimum_session_duration")
							  .Help("Minimum duration of sessions")
							  .Register(*reg)
							  .Add({});

	// Set defaults
	minProcessingTime->Set(std::numeric_limits<double>::max());
	minSessionDuration->Set(std::numeric_limits<double>::max());
}

void TelnetStats::consumeStats(const TelnetSessionStats &stat, bool sessionClosed)
{
	succeededCommand->Increment(static_cast<double>(stat.successCmdCtr));
	failedCommand->Increment(static_cast<double>(stat.failCmdCtr));
	totalCommand->Increment(static_cast<double>(stat.successCmdCtr + stat.failCmdCtr));

	totalUploadBytes->Increment(static_cast<double>(stat.uploadBytes));
	totalDownloadBytes->Increment(static_cast<double>(stat.downloadBytes));

	if (sessionClosed)
	{
		// Session durations
		const auto sessionTime = static_cast<double>(
			std::chrono::duration_cast<std::chrono::seconds>(stat.disconnectTime - stat.connectTime).count());
		sessionDuration->Observe(sessionTime);
		if (sessionTime < minSessionDuration->Value())
		{
			minSessionDuration->Set(sessionTime);
		}
		if (sessionTime > maxSessionDuration->Value())
		{
			maxSessionDuration->Set(sessionTime);
		}
	}
}

void TelnetStats::consumeStats(const TelnetServerStats &stat)
{
	// Connection stats
	activeConnection->Set(static_cast<double>(stat.activeConnectionCtr));
	refusedConnection->Increment(static_cast<double>(stat.refusedConnectionCtr));
	totalConnection->Increment(static_cast<double>(stat.acceptedConnectionCtr));

	// Performance stats if there is an active connection
	if (stat.activeConnectionCtr > 0)
	{
		const auto processTime = static_cast<double>((stat.processingTimeEnd - stat.processingTimeStart).count());
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
}
