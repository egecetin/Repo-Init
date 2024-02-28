#include "zeromq/ZeroMQMonitor.hpp"

#include <spdlog/spdlog.h>

constexpr int EVENT_CHECK_TIMEOUT_MS = 100;

void ZeroMQMonitor::threadFunc()
{
	while (!shouldStop)
	{
		check_event(EVENT_CHECK_TIMEOUT_MS);
	}
}

void ZeroMQMonitor::on_event(const std::string &messageStr, int level, const char *addr)
{
	switch (level)
	{
	case spdlog::level::trace:
		spdlog::trace("{} {}", messageStr, addr == nullptr ? "" : addr);
		break;
	case spdlog::level::debug:
		spdlog::debug("{} {}", messageStr, addr == nullptr ? "" : addr);
		break;
	case spdlog::level::info:
		spdlog::info("{} {}", messageStr, addr == nullptr ? "" : addr);
		break;
	case spdlog::level::warn:
		spdlog::warn("{} {}", messageStr, addr == nullptr ? "" : addr);
		break;
	case spdlog::level::err:
		spdlog::error("{} {}", messageStr, addr == nullptr ? "" : addr);
		break;
	case spdlog::level::critical:
		spdlog::critical("{} {}", messageStr, addr == nullptr ? "" : addr);
		break;
	default:
		spdlog::warn("Unknwon log level {} {} {}", messageStr, addr == nullptr ? "" : addr, level);
		break;
	}
}

void ZeroMQMonitor::on_monitor_started() { on_event("Monitor started", spdlog::level::info); }

void ZeroMQMonitor::on_event_connected(const zmq_event_t &, const char *addr_)
{
	on_event("Connected", spdlog::level::info, addr_);
}

void ZeroMQMonitor::on_event_connect_delayed(const zmq_event_t &, const char *addr_)
{
	on_event("Connect delayed", spdlog::level::debug, addr_);
}

void ZeroMQMonitor::on_event_connect_retried(const zmq_event_t &, const char *addr_)
{
	on_event("Connect retried", spdlog::level::debug, addr_);
}

void ZeroMQMonitor::on_event_listening(const zmq_event_t &, const char *addr_)
{
	on_event("Listening", spdlog::level::debug, addr_);
}

void ZeroMQMonitor::on_event_bind_failed(const zmq_event_t &, const char *addr_)
{
	on_event("Bind failed", spdlog::level::warn, addr_);
}

void ZeroMQMonitor::on_event_accepted(const zmq_event_t &, const char *addr_)
{
	on_event("Accepted", spdlog::level::info, addr_);
}

void ZeroMQMonitor::on_event_accept_failed(const zmq_event_t &, const char *addr_)
{
	on_event("Accept failed", spdlog::level::warn, addr_);
}

void ZeroMQMonitor::on_event_closed(const zmq_event_t &, const char *addr_)
{
	on_event("Closed", spdlog::level::debug, addr_);
}

void ZeroMQMonitor::on_event_close_failed(const zmq_event_t &, const char *addr_)
{
	on_event("Close failed", spdlog::level::warn, addr_);
}

void ZeroMQMonitor::on_event_disconnected(const zmq_event_t &, const char *addr_)
{
	on_event("Disconnected", spdlog::level::info, addr_);
}

void ZeroMQMonitor::on_event_handshake_failed_no_detail(const zmq_event_t &, const char *addr_)
{
	on_event("Handshake failed (no detail)", spdlog::level::warn, addr_);
}

void ZeroMQMonitor::on_event_handshake_failed_protocol(const zmq_event_t &, const char *addr_)
{
	on_event("Handshake failed (protocol)", spdlog::level::warn, addr_);
}

void ZeroMQMonitor::on_event_handshake_failed_auth(const zmq_event_t &, const char *addr_)
{
	on_event("Handshake failed (auth)", spdlog::level::warn, addr_);
}

void ZeroMQMonitor::on_event_handshake_succeeded(const zmq_event_t &, const char *addr_)
{
	on_event("Handshake succeeded", spdlog::level::info, addr_);
}

void ZeroMQMonitor::on_event_handshake_failed(const zmq_event_t &, const char *addr_)
{
	on_event("Handshake failed", spdlog::level::warn, addr_);
}

void ZeroMQMonitor::on_event_handshake_succeed(const zmq_event_t &, const char *addr_)
{
	on_event("Handshake succeed", spdlog::level::info, addr_);
}

void ZeroMQMonitor::on_event_unknown(const zmq_event_t &, const char *addr_)
{
	on_event("Unknown event", spdlog::level::warn, addr_);
}

void ZeroMQMonitor::startMonitoring(const std::unique_ptr<zmq::socket_t> &socket, const std::string &monitorAddress)
{
	init(*socket, monitorAddress);
	monitorThread = std::thread(&ZeroMQMonitor::threadFunc, this);
}

void ZeroMQMonitor::stopMonitoring()
{
	shouldStop = true;
	abort();

	// Join the thread
	if (monitorThread.joinable())
	{
		monitorThread.join();
	}
}
