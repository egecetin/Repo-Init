#include "connection/Zeromq.hpp"

#include <atomic>
#include <thread>

#include <spdlog/spdlog.h>

constexpr int EVENT_CHECK_TIMEOUT_MS = 100;

class ZeroMQMonitor : private zmq::monitor_t {
  private:
	std::string _name;
	std::thread monitorThread;
	std::atomic<bool> shouldStop{false};

	void threadFunc()
	{
		while (!shouldStop)
		{
			check_event(EVENT_CHECK_TIMEOUT_MS);
		}
	}

	void on_monitor_started() { spdlog::info("Monitor started for {}", _name); }

	void on_event_connected(const zmq_event_t &event_, const char *addr_) { spdlog::debug("Connected to {}", addr_); }

	void on_event_connect_delayed(const zmq_event_t &event_, const char *addr_)
	{
		spdlog::debug("Connect delayed to {}", addr_);
	}

	void on_event_connect_retried(const zmq_event_t &event_, const char *addr_)
	{
		spdlog::debug("Connect retried to {}", addr_);
	}

	void on_event_listening(const zmq_event_t &event_, const char *addr_) { spdlog::debug("Listening on {}", addr_); }

	void on_event_bind_failed(const zmq_event_t &event_, const char *addr_)
	{
		spdlog::warn("Bind failed on {}", addr_);
	}

	void on_event_accepted(const zmq_event_t &event_, const char *addr_) { spdlog::info("Accepted on {}", addr_); }

	void on_event_accept_failed(const zmq_event_t &event_, const char *addr_)
	{
		spdlog::warn("Accept failed on {}", addr_);
	}

	void on_event_closed(const zmq_event_t &event_, const char *addr_) { spdlog::info("Closed on {}", addr_); }

	void on_event_close_failed(const zmq_event_t &event_, const char *addr_)
	{
		spdlog::warn("Close failed on {}", addr_);
	}

	void on_event_disconnected(const zmq_event_t &event_, const char *addr_)
	{
		spdlog::warn("Disconnected from {}", addr_);
	}

	void on_event_handshake_failed_no_detail(const zmq_event_t &event_, const char *addr_)
	{
		spdlog::warn("Handshake failed (no detail) on {}", addr_);
	}

	void on_event_handshake_failed_protocol(const zmq_event_t &event_, const char *addr_)
	{
		spdlog::warn("Handshake failed (protocol) on {}", addr_);
	}

	void on_event_handshake_failed_auth(const zmq_event_t &event_, const char *addr_)
	{
		spdlog::warn("Handshake failed (auth) on {}", addr_);
	}

	void on_event_handshake_succeeded(const zmq_event_t &event_, const char *addr_)
	{
		spdlog::debug("Handshake succeeded on {}", addr_);
	}

	void on_event_handshake_failed(const zmq_event_t &event_, const char *addr_)
	{
		spdlog::warn("Handshake failed on {}", addr_);
	}

	void on_event_handshake_succeed(const zmq_event_t &event_, const char *addr_)
	{
		spdlog::debug("Handshake succeed on {}", addr_);
	}

	void on_event_unknown(const zmq_event_t &event_, const char *addr_) { spdlog::debug("Unknown event on {}", addr_); }

  public:
	explicit ZeroMQMonitor(const std::string name = "") : _name(name) {}

	void startMonitoring(const std::unique_ptr<zmq::socket_t> &socket, const std::string &monitorAddress)
	{
		init(*socket, monitorAddress);
		monitorThread = std::thread(&ZeroMQMonitor::threadFunc, this);
	}

	void stopMonitoring()
	{
		shouldStop = true;
		abort();

		// Join the thread
		if (monitorThread.joinable())
		{
			monitorThread.join();
		}
	}
};
