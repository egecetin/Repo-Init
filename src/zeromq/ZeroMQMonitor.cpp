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
		if (monitorThread.joinable())
		{
			monitorThread.join();
		}
	}
};
