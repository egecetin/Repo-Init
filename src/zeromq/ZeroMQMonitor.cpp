#include "zeromq/ZeroMQMonitor.hpp"

#include <spdlog/spdlog.h>

constexpr int EVENT_CHECK_TIMEOUT_MS = 100;

void ZeroMQMonitor::threadFunc()
{
	while (!_shouldStop._M_i)
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
		spdlog::warn("Unknown log level {} {} {}", messageStr, addr == nullptr ? "" : addr, level);
		break;
	}
}

void ZeroMQMonitor::on_monitor_started() { on_event("Monitor started", spdlog::level::info); }

void ZeroMQMonitor::on_event_connected(const zmq_event_t & /*unused*/, const char *addr_)
{
	_peerCount.fetch_add(1);
	on_event("Connected", spdlog::level::info, addr_);
}

void ZeroMQMonitor::on_event_connect_delayed(const zmq_event_t & /*unused*/, const char *addr_)
{
	on_event("Connect delayed", spdlog::level::debug, addr_);
}

void ZeroMQMonitor::on_event_connect_retried(const zmq_event_t & /*unused*/, const char *addr_)
{
	on_event("Connect retried", spdlog::level::debug, addr_);
}

void ZeroMQMonitor::on_event_listening(const zmq_event_t & /*unused*/, const char *addr_)
{
	on_event("Listening", spdlog::level::debug, addr_);
}

void ZeroMQMonitor::on_event_bind_failed(const zmq_event_t & /*unused*/, const char *addr_)
{
	on_event("Bind failed", spdlog::level::warn, addr_);
}

void ZeroMQMonitor::on_event_accepted(const zmq_event_t & /*unused*/, const char *addr_)
{
	on_event("Accepted", spdlog::level::info, addr_);
}

void ZeroMQMonitor::on_event_accept_failed(const zmq_event_t & /*unused*/, const char *addr_)
{
	on_event("Accept failed", spdlog::level::warn, addr_);
}

void ZeroMQMonitor::on_event_closed(const zmq_event_t & /*unused*/, const char *addr_)
{
	on_event("Closed", spdlog::level::debug, addr_);
}

void ZeroMQMonitor::on_event_close_failed(const zmq_event_t & /*unused*/, const char *addr_)
{
	on_event("Close failed", spdlog::level::warn, addr_);
}

void ZeroMQMonitor::on_event_disconnected(const zmq_event_t & /*unused*/, const char *addr_)
{
	_peerCount.fetch_sub(1);
	on_event("Disconnected", spdlog::level::info, addr_);
}

#if ZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 3, 0) ||                                                                        \
	(defined(ZMQ_BUILD_DRAFT_API) && ZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 2, 3))
void ZeroMQMonitor::on_event_handshake_failed_no_detail(const zmq_event_t & /*unused*/, const char *addr_)
{
	on_event("Handshake failed (no detail)", spdlog::level::warn, addr_);
}

void ZeroMQMonitor::on_event_handshake_failed_protocol(const zmq_event_t & /*unused*/, const char *addr_)
{
	on_event("Handshake failed (protocol)", spdlog::level::warn, addr_);
}

void ZeroMQMonitor::on_event_handshake_failed_auth(const zmq_event_t & /*unused*/, const char *addr_)
{
	on_event("Handshake failed (auth)", spdlog::level::warn, addr_);
}

void ZeroMQMonitor::on_event_handshake_succeeded(const zmq_event_t & /*unused*/, const char *addr_)
{
	on_event("Handshake succeeded", spdlog::level::info, addr_);
}

#elif defined(ZMQ_BUILD_DRAFT_API) && ZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 2, 1)
void ZeroMQMonitor::on_event_handshake_failed(const zmq_event_t & /*unused*/, const char *addr_)
{
	on_event("Handshake failed", spdlog::level::warn, addr_);
}

void ZeroMQMonitor::on_event_handshake_succeed(const zmq_event_t & /*unused*/, const char *addr_)
{
	on_event("Handshake succeed", spdlog::level::info, addr_);
}
#endif

void ZeroMQMonitor::on_event_unknown(const zmq_event_t & /*unused*/, const char *addr_)
{
	on_event("Unknown event", spdlog::level::warn, addr_);
}

void ZeroMQMonitor::startMonitoring(zmq::socket_t *socket, const std::string &monitorAddress)
{
	if (socket == nullptr)
	{
		throw std::invalid_argument("ZeroMQ socket to monitor is nullptr");
	}

	init(*socket, monitorAddress);
	_monitorThread = std::make_unique<std::thread>(&ZeroMQMonitor::threadFunc, this);
}

void ZeroMQMonitor::stopMonitoring()
{
	_shouldStop.test_and_set();
#ifdef ZMQ_EVENT_MONITOR_STOPPED
	abort();
#endif

	// Join the thread
	if (_monitorThread && _monitorThread->joinable())
	{
		_monitorThread->join();
		_monitorThread.reset();
	}

	spdlog::info("Monitor stopped");
}
