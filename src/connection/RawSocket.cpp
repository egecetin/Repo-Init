#include "connection/RawSocket.hpp"

#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <stdexcept>

RawSocket::RawSocket(const std::string &iface, bool isWrite)
{
	// Set variables
	iFace = iface;
	sockFd = -1;
	writeMode = isWrite;
	isReady = false;

	// Prepare socket address
	memset((void *)&addr, 0, sizeof(struct sockaddr_ll));
	addr.sll_family = AF_PACKET;
	addr.sll_protocol = htons(ETH_P_ALL);
	addr.sll_ifindex = if_nametoindex(iFace.c_str());
	if (!addr.sll_ifindex)
		throw std::runtime_error(std::string("Can't find interface: ") + strerror(errno));

	// Interface request
	struct ifreq ifr;
	memset((void *)&ifr, 0, sizeof(struct ifreq));
	memcpy(ifr.ifr_name, iFace.c_str(), iFace.size()); // Size should be sufficient because if_nametoindex not failed

	if (isWrite)
	{
		sockFd = socket(PF_PACKET, SOCK_RAW, IPPROTO_RAW); // Init socket
		if (sockFd < 0)
			throw std::runtime_error(strerror(errno));
		if (bind(sockFd, (struct sockaddr *)&addr, sizeof(addr)) < 0) // Bind to interface
			throw std::runtime_error(std::string("Bind failed: ") + strerror(errno));
		if (setsockopt(sockFd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) // Set socket options
			throw std::runtime_error(std::string("Can't set socket options: ") + strerror(errno));
	}
	else
	{
		sockFd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)); // Init socket
		if (sockFd < 0)
			throw std::runtime_error(strerror(errno));
		if (bind(sockFd, (struct sockaddr *)&addr, sizeof(addr)) < 0) // Bind to interface
			throw std::runtime_error(std::string("Bind failed: ") + strerror(errno));
	}
	isReady = true;
}

int RawSocket::writeData(const void *data, size_t dataLen)
{
	if (!isReady || !writeMode)
		return -EPERM;

	auto startTime = std::chrono::high_resolution_clock::now();
	int retval = write(sockFd, data, dataLen);

	// Update stats
	stats.processingTime += (std::chrono::high_resolution_clock::now() - startTime).count();
	stats.sentBytes += dataLen;

	return retval;
}

int RawSocket::readData(void *data, size_t dataLen)
{
	if (!isReady || writeMode)
		return -EPERM;
	socklen_t socketLen = sizeof(addr);

	auto startTime = std::chrono::high_resolution_clock::now();
	int retval = recvfrom(sockFd, data, dataLen, 0, (struct sockaddr *)&addr, &socketLen);

	// Update stats
	stats.processingTime += (std::chrono::high_resolution_clock::now() - startTime).count();
	stats.receivedBytes += dataLen;

	return retval;
}

RawSocketStats RawSocket::getStats(bool resetInternalStats)
{
	if (resetInternalStats)
	{
		RawSocketStats buffer = stats;
		stats = {0, 0, 0.0};
		return buffer;
	}
	return stats;
}

RawSocket::~RawSocket() { close(sockFd); }
