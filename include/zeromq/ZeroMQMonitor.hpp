#pragma once

#include <atomic>
#include <string>
#include <thread>

#include <zmq.hpp>

/**
 * @class ZeroMQMonitor
 * @brief Class for monitoring ZeroMQ events on a socket.
 *
 * The ZeroMQMonitor class provides functionality to monitor ZeroMQ events on a socket.
 * It starts a separate thread to listen for events and invokes corresponding event handlers.
 * The class supports various event types such as connection, binding, acceptance, closure, etc.
 */
class ZeroMQMonitor : private zmq::monitor_t {
  private:
	std::thread monitorThread; /**< Thread for monitoring events. */
	std::atomic<bool> shouldStop{false}; /**< Flag to stop monitoring. */

	void threadFunc();

	void on_event(const std::string &messageStr, int level, const char *addr = nullptr);

	void on_monitor_started();

	void on_event_connected(const zmq_event_t &, const char *addr_);

	void on_event_connect_delayed(const zmq_event_t &, const char *addr_);

	void on_event_connect_retried(const zmq_event_t &, const char *addr_);

	void on_event_listening(const zmq_event_t &, const char *addr_);

	void on_event_bind_failed(const zmq_event_t &, const char *addr_);

	void on_event_accepted(const zmq_event_t &, const char *addr_);

	void on_event_accept_failed(const zmq_event_t &, const char *addr_);

	void on_event_closed(const zmq_event_t &, const char *addr_);

	void on_event_close_failed(const zmq_event_t &, const char *addr_);

	void on_event_disconnected(const zmq_event_t &, const char *addr_);

	void on_event_handshake_failed_no_detail(const zmq_event_t &, const char *addr_);

	void on_event_handshake_failed_protocol(const zmq_event_t &, const char *addr_);

	void on_event_handshake_failed_auth(const zmq_event_t &, const char *addr_);

	void on_event_handshake_succeeded(const zmq_event_t &, const char *addr_);

	void on_event_handshake_failed(const zmq_event_t &, const char *addr_);

	void on_event_handshake_succeed(const zmq_event_t &, const char *addr_);

	void on_event_unknown(const zmq_event_t &, const char *addr_);

  public:
    /**
     * @brief Start monitoring events on the given socket.
     * 
     * This method starts a separate thread to listen for events on the given socket.
     * The method also sets up the event handlers for various event types.
     * 
     * @param[in] socket Zeromq socket
     * @param[in] monitorAddress Monitoring address
     */
	void startMonitoring(const std::unique_ptr<zmq::socket_t> &socket, const std::string &monitorAddress);

    /**
     * @brief Stop monitoring events on the socket.
     */
	void stopMonitoring();
};