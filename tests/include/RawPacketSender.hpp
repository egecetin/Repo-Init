#pragma once

#include <linux/if_packet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <stdexcept>
#include <string>
#include <thread>

/**
 * @class RawPacketSender
 * A simple raw ethernet packet sender for testing purposes
 * Sends raw packets on a specified network interface
 */
class RawPacketSender {
  private:
	int _sockfd{-1};
	std::jthread _senderThread;

	/**
	 * Sender loop that transmits packets
	 * @param[in] message The message to send
	 * @param[in] count Number of packets to send
	 * @param[in] delayMs Delay between packets in milliseconds
	 * @param[in] stopToken Stop token for cooperative cancellation
	 */
	void senderLoop(std::string message, int count, int delayMs, std::stop_token stopToken)
	{
		for (int i = 0; i < count && !stopToken.stop_requested(); ++i)
		{
			ssize_t sent = send(_sockfd, message.c_str(), message.length(), 0);
			if (sent < 0)
			{
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
		}
	}

  public:
	/**
	 * Constructs a new RawPacketSender object
	 * @param[in] interface The network interface name (e.g., "eth0")
	 * @param[in] message The message to send in each packet
	 * @param[in] count Number of packets to send (default: 10)
	 * @param[in] delayMs Delay between packets in milliseconds (default: 100)
	 * @throws std::runtime_error if socket creation or binding fails
	 */
	explicit RawPacketSender(const std::string &interface, const std::string &message, int count = 10, int delayMs = 100)
	{
		// Create raw socket
		_sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
		if (_sockfd < 0)
		{
			throw std::runtime_error("Failed to create raw socket");
		}

		// Get interface index
		struct ifreq ifr;
		memset(&ifr, 0, sizeof(ifr));
		strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ - 1);
		if (ioctl(_sockfd, SIOCGIFINDEX, &ifr) < 0)
		{
			close(_sockfd);
			throw std::runtime_error("Failed to get interface index");
		}

		// Bind socket to interface
		struct sockaddr_ll addr;
		memset(&addr, 0, sizeof(addr));
		addr.sll_family = AF_PACKET;
		addr.sll_protocol = htons(ETH_P_ALL);
		addr.sll_ifindex = ifr.ifr_ifindex;

		if (bind(_sockfd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0)
		{
			close(_sockfd);
			throw std::runtime_error("Failed to bind socket to interface");
		}

		// Start sender thread
		_senderThread = std::jthread([this, message, count, delayMs](std::stop_token stopToken) {
			this->senderLoop(message, count, delayMs, stopToken);
		});
	}

	/**
	 * Destructor - cleans up socket and stops sender thread
	 */
	~RawPacketSender()
	{
		if (_senderThread.joinable())
		{
			_senderThread.request_stop();
			_senderThread.join();
		}

		if (_sockfd >= 0)
		{
			close(_sockfd);
		}
	}

	// Delete copy constructor and assignment operator
	RawPacketSender(const RawPacketSender &) = delete;
	RawPacketSender &operator=(const RawPacketSender &) = delete;
};
