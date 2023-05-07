#include "connection/RawSocket.hpp"
#include "Utils.hpp"

#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <stdexcept>
#include <utility>

RawSocket::RawSocket(std::string iface, bool isWrite) : writeMode(isWrite), iFace(std::move(iface))
{
	// Prepare socket address
	memset((void *)&addr, 0, sizeof(sockaddr_ll));
	addr.sll_family = AF_PACKET;
	addr.sll_protocol = htons(ETH_P_ALL);
	addr.sll_ifindex = static_cast<int>(if_nametoindex(iFace.c_str()));
	if (addr.sll_ifindex == 0)
	{
		throw std::runtime_error(std::string("Can't find interface: ") + getErrnoString(errno));
	}

	// Interface request
	ifreq ifr{};
	memset((void *)&ifr, 0, sizeof(ifreq));
	memcpy(ifr.ifr_name, iFace.c_str(), iFace.size()); // Size should be sufficient because if_nametoindex not failed

	if (isWrite)
	{
		sockFd = socket(PF_PACKET, SOCK_RAW, IPPROTO_RAW); // Init socket
		if (sockFd < 0)
		{
			throw std::runtime_error(getErrnoString(errno));
		}
		if (bind(sockFd, (sockaddr *)&addr, sizeof(addr)) < 0)
		{
			throw std::runtime_error(std::string("Bind failed: ") + getErrnoString(errno));
		}
		if (setsockopt(sockFd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0)
		{ // Set socket options
			throw std::runtime_error(std::string("Can't set socket options: ") + getErrnoString(errno));
		}
	}
	else
	{
		sockFd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)); // Init socket
		if (sockFd < 0)
		{
			throw std::runtime_error(getErrnoString(errno));
		}
		if (bind(sockFd, (sockaddr *)&addr, sizeof(addr)) < 0)
		{
			throw std::runtime_error(std::string("Bind failed: ") + getErrnoString(errno));
		}
	}
	isReady = true;
}

int RawSocket::writeData(const void *data, size_t dataLen)
{
	if (!isReady || !writeMode)
	{
		return -EPERM;
	}

	auto startTime = std::chrono::high_resolution_clock::now();
	const int retval = static_cast<int>(write(sockFd, data, dataLen));

	// Update stats
	stats.processingTime += static_cast<double>((std::chrono::high_resolution_clock::now() - startTime).count());
	stats.sentBytes += dataLen;

	return retval;
}

int RawSocket::readData(void *data, size_t dataLen)
{
	if (!isReady || writeMode)
	{
		return -EPERM;
	}
	socklen_t socketLen = sizeof(addr);

	auto startTime = std::chrono::high_resolution_clock::now();
	const int retval = static_cast<int>(recvfrom(sockFd, data, dataLen, 0, (sockaddr *)&addr, &socketLen));

	// Update stats
	stats.processingTime += static_cast<double>((std::chrono::high_resolution_clock::now() - startTime).count());
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
