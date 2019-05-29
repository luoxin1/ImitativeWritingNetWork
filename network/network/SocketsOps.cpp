#include "SocketsOps.h"
#include <sys/socket.h>
#include "boost/implicit_cast.hpp"
#include<iostream>
#include <string.h>
#include"Endian.h"
#include<assert.h>

const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in* addr)
{
	return static_cast<const struct sockaddr*>(boost::implicit_cast<const void*>(addr));
}

const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in6* addr)
{
	return static_cast<const struct sockaddr*>(boost::implicit_cast<const void*>(addr));
}

struct sockaddr* sockets::sockaddr_cast(struct sockaddr_in6* addr)
{
	return static_cast<struct sockaddr*>(boost::implicit_cast<void*>(addr));
}

const struct sockaddr_in* sockets::sockaddr_in_cast(const struct sockaddr* addr)
{
	return static_cast<const struct sockaddr_in*>(boost::implicit_cast<const void*>(addr));
}

const struct sockaddr_in6* sockets::sockaddr_in6_cast(const struct sockaddr* addr)
{
	return static_cast<const struct sockaddr_in6*>(boost::implicit_cast<const void*>(addr));
}

void sockets::setOption(int fd, int opt, int level, int optval)
{
	switch (level)
	{
	case SOL_SOCKET:
	case IPPROTO_TCP:
		setsockopt(fd, level, opt, &optval, static_cast<socklen_t>(sizeof(optval)));
		break;
	default:
		break;
	}
}

void sockets::shutdownWrite(int sockfd)
{
	if (::shutdown(sockfd,SHUT_WR)<0)
	{
		std::cout << "socket::shutdownWrite" << std::endl;
	}
}

void sockets::toIpPort(char* buf, size_t size, const struct sockaddr* addr)
{
	toIp(buf, size, addr);
	size_t end = strlen(buf);
	const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
	uint16_t port = sockets::networkToHost16(addr4->sin_port);
	assert(size > end);
	snprintf(buf + end, size - end, ":%u", port);
}

void sockets::toIp(char* buf, size_t size, const struct sockaddr* addr)
{
	if (addr->sa_family==AF_INET)
	{
		assert(size >= INET_ADDRSTRLEN);
		const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
		::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
	}
	else if (addr->sa_family==AF_INET6)
	{
		assert(size >= INET6_ADDRSTRLEN);
		const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
		::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
	}
}

void sockets::fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr)
{
	addr->sin_family= AF_INET;
	addr->sin_port = hostToNetwork16(port);
	if (inet_pton(AF_INET,ip,&addr->sin_addr) <=0 )
	{
		std::cout << "socket::fromIpPort" << std::endl;
	}
}

void sockets::fromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr)
{
	addr->sin6_family = AF_INET6;
	addr->sin6_port = hostToNetwork16(port);
	if (inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0)
	{
		std::cout << "socket::fromIpPort" << std::endl;
	}	
}

struct sockaddr_in6 sockets::getLocalAddress(int sockfd)
{
	struct sockaddr_in6 local;
	bzero(&local, sizeof(local));
	socklen_t addrlen = static_cast<socklen_t>(sizeof(local));
	if (getsockname(sockfd, sockaddr_cast(&local), &addrlen) < 0)
	{
		std::cout << "sockets::getLocalAddress" << std::endl;
	}
	return local;
}

struct sockaddr_in6 sockets::getRemoteAddress(int sockfd)
{
	struct sockaddr_in6 remote;
	bzero(&remote, sizeof(remote));
	socklen_t addrlen = static_cast<socklen_t>(sizeof(remote));
	if (getpeername(sockfd, sockaddr_cast(&remote), &addrlen) < 0)
	{
		std::cout << "sockets::getRemoteAddress" << std::endl;
	}
	return remote;
}

bool sockets::isSelfConnect(int sockfd)
{
	struct sockaddr_in6 local = getLocalAddress(sockfd);
	struct sockaddr_in6 remote = getRemoteAddress(sockfd);
	if (local.sin6_family ==AF_INET)
	{
		const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&local);
		const struct sockaddr_in* raddr4 = reinterpret_cast<struct sockaddr_in*>(&remote);
		return laddr4->sin_port == raddr4->sin_port &&
			laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
	}
	else if (local.sin6_family==AF_INET6)
	{
		return local.sin6_port == remote.sin6_port &&
			memcmp(&local.sin6_addr, &remote.sin6_addr, sizeof(local.sin6_addr)) == 0;
	}
	return false;
}