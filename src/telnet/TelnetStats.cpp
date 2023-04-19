#include "telnet/TelnetStats.hpp"

#include <limits>

#include <date/date.h>

TelnetStats::TelnetStats(std::shared_ptr<prometheus::Registry> reg, uint16_t portNumber)
{
	if (!reg)
		throw std::runtime_error("Can't init Telnet statistics. Registry is null");

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
						  .Add({}, prometheus::Summary::Quantiles{{0.5, 0.1}, {0.9, 0.1}, {0.99, 0.1}});
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
						   .Add({}, prometheus::Summary::Quantiles{{0.5, 0.1}, {0.9, 0.1}, {0.99, 0.1}});
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

void TelnetStats::consumeStats(TelnetSessionStats &stat, bool sessionClosed)
{
	succeededCommand->Increment(stat.successCmdCtr);
	failedCommand->Increment(stat.failCmdCtr);
	totalCommand->Increment(stat.successCmdCtr + stat.failCmdCtr);

	totalUploadBytes->Increment(stat.uploadBytes);
	totalDownloadBytes->Increment(stat.downloadBytes);

	if (sessionClosed)
	{
		// Session durations
		double sessionTime =
			std::chrono::duration_cast<std::chrono::seconds>(stat.disconnectTime - stat.connectTime).count();
		sessionDuration->Observe(sessionTime);
		if (sessionTime < minSessionDuration->Value())
			minSessionDuration->Set(sessionTime);
		if (sessionTime > maxSessionDuration->Value())
			maxSessionDuration->Set(sessionTime);
	}
}

void TelnetStats::consumeStats(TelnetServerStats &stat)
{
	// Connection stats
	activeConnection->Set(stat.activeConnectionCtr);
	refusedConnection->Increment(stat.refusedConnectionCtr);
	totalConnection->Increment(stat.acceptedConnectionCtr);

	// Performance stats if there is an active connection
	if (stat.activeConnectionCtr)
	{
		double processTime = (stat.processingTimeEnd - stat.processingTimeStart).count();
		processingTime->Observe(processTime);
		if (processTime < minProcessingTime->Value())
			minProcessingTime->Set(processTime);
		if (processTime > maxProcessingTime->Value())
			maxProcessingTime->Set(processTime);
	}
}
