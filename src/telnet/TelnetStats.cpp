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
		throw std::invalid_argument("Can't init Telnet statistics. Registry is null");
	}

	// Basic information
	_infoFamily = &prometheus::BuildInfo().Name("telnet").Help("Telnet server information").Register(*reg);

	_infoFamily->Add({{"server_port", std::to_string(portNumber)}});
	_infoFamily->Add({{"init_time", date::format("%FT%TZ", date::floor<std::chrono::nanoseconds>(
															  std::chrono::high_resolution_clock::now()))}});
	_infoFamily->Add({{"performance_unit", "nanoseconds"}});

	// Connection stats
	_activeConnection = &prometheus::BuildGauge()
							.Name("telnet_active_connections")
							.Help("Number of active connections")
							.Register(*reg)
							.Add({});
	_refusedConnection = &prometheus::BuildCounter()
							 .Name("telnet_refused_connections")
							 .Help("Number of refused connections")
							 .Register(*reg)
							 .Add({});
	_totalConnection = &prometheus::BuildCounter()
						   .Name("telnet_received_connections")
						   .Help("Number of received connections")
						   .Register(*reg)
						   .Add({});

	// Performance stats
	_processingTime = &prometheus::BuildSummary()
						  .Name("telnet_processing_time")
						  .Help("Command processing performance")
						  .Register(*reg)
						  .Add({}, QUANTILE_DEFAULTS);
	_maxProcessingTime = &prometheus::BuildGauge()
							 .Name("telnet_maximum_processing_time")
							 .Help("Maximum value of the command processing performance")
							 .Register(*reg)
							 .Add({});
	_minProcessingTime = &prometheus::BuildGauge()
							 .Name("telnet_minimum_processing_time")
							 .Help("Minimum value of the command processing performance")
							 .Register(*reg)
							 .Add({});

	// Command stats
	_succeededCommand = &prometheus::BuildCounter()
							.Name("telnet_succeeded_commands")
							.Help("Number of succeeded commands")
							.Register(*reg)
							.Add({});
	_failedCommand = &prometheus::BuildCounter()
						 .Name("telnet_failed_commands")
						 .Help("Number of failed commands")
						 .Register(*reg)
						 .Add({});
	_totalCommand = &prometheus::BuildCounter()
						.Name("telnet_received_commands")
						.Help("Number of received commands")
						.Register(*reg)
						.Add({});

	// Bandwidth stats
	_totalUploadBytes =
		&prometheus::BuildCounter().Name("telnet_uploaded_bytes").Help("Total uploaded bytes").Register(*reg).Add({});
	_totalDownloadBytes = &prometheus::BuildCounter()
							  .Name("telnet_downloaded_bytes")
							  .Help("Total downloaded bytes")
							  .Register(*reg)
							  .Add({});

	// Session durations
	_sessionDuration = &prometheus::BuildSummary()
						   .Name("telnet_session_duration")
						   .Help("Duration of sessions")
						   .Register(*reg)
						   .Add({}, QUANTILE_DEFAULTS);
	_maxSessionDuration = &prometheus::BuildGauge()
							  .Name("telnet_maximum_session_duration")
							  .Help("Maximum duration of sessions")
							  .Register(*reg)
							  .Add({});
	_minSessionDuration = &prometheus::BuildGauge()
							  .Name("telnet_minimum_session_duration")
							  .Help("Minimum duration of sessions")
							  .Register(*reg)
							  .Add({});

	// Set defaults
	_minProcessingTime->Set(std::numeric_limits<double>::max());
	_minSessionDuration->Set(std::numeric_limits<double>::max());
}

void TelnetStats::consumeStats(const TelnetSessionStats &stat, bool sessionClosed)
{
	_succeededCommand->Increment(static_cast<double>(stat.successCmdCtr));
	_failedCommand->Increment(static_cast<double>(stat.failCmdCtr));
	_totalCommand->Increment(static_cast<double>(stat.successCmdCtr + stat.failCmdCtr));

	_totalUploadBytes->Increment(static_cast<double>(stat.uploadBytes));
	_totalDownloadBytes->Increment(static_cast<double>(stat.downloadBytes));

	if (sessionClosed)
	{
		// Session durations
		const auto sessionTime = static_cast<double>(
			std::chrono::duration_cast<std::chrono::seconds>(stat.disconnectTime - stat.connectTime).count());
		_sessionDuration->Observe(sessionTime);
		if (sessionTime < _minSessionDuration->Value())
		{
			_minSessionDuration->Set(sessionTime);
		}
		if (sessionTime > _maxSessionDuration->Value())
		{
			_maxSessionDuration->Set(sessionTime);
		}
	}
}

void TelnetStats::consumeStats(const TelnetServerStats &stat)
{
	// Connection stats
	_activeConnection->Set(static_cast<double>(stat.activeConnectionCtr));
	_refusedConnection->Increment(static_cast<double>(stat.refusedConnectionCtr));
	_totalConnection->Increment(static_cast<double>(stat.acceptedConnectionCtr));

	// Performance stats if there is an active connection
	if (stat.activeConnectionCtr > 0)
	{
		const auto processTime = static_cast<double>((stat.processingTimeEnd - stat.processingTimeStart).count());
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
}
