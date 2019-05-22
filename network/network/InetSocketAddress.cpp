#include "boost/static_assert.hpp"
#include "InetSocketAddress.h"
#include <assert.h>
#include <netdb.h>
#include <strings.h> 
#include <netinet/in.h>
#include<iostream>
#include"Endian.h"

static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
static const in_addr_t kInaddrAny = INADDR_ANY;

//BOOST_STATIC_ASSERTT(sizeof(InetSocketAddress) == sizeof(struct sockaddr_in6));
//BOOST_STATIC_ASSERT(offsetof(sockaddr_in,sin_family)==0);
//BOOST_STATIC_ASSERT(offsetof(sockaddr_in6,sin6_family)==0);
//BOOST_STATIC_ASSERT(offsetof(sockaddr_in,sin_port)==2);
//BOOST_STATIC_ASSERT(offsetof(sockaddr_in6,sin6_port)==2);


InetSocketAddress::InetSocketAddress(uint16_t port , bool loopbackOnly , bool ipv6)
{
//	BOOST_STATIC_ASSERT(offsetof(InetSocketAddress, addr6_) == 0);
//	BOOST_STATIC_ASSERT(offsetof(InetSocketAddress, addr_) == 0);

	if (ipv6)
	{
		bzero(&addr6_, sizeof(addr6_));
		addr6_.sin6_family = AF_INET6;
		in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
		addr6_.sin6_addr = ip;
		addr6_.sin6_port = sockets::hostToNetwork16(port);
	}
	else
	{
		bzero(&addr_, sizeof(addr_));
		addr_.sin_family = AF_INET;
		in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
		addr_.sin_addr.s_addr = sockets::hostToNetwork32(ip);
		addr_.sin_port = sockets::hostToNetwork16(port);
	}
}

InetSocketAddress::InetSocketAddress(const std::string& ip, uint16_t port, bool ipv6)
{
	if (ipv6)
	{
		bzero(&addr6_, sizeof(addr6_));
		sockets::fromIpPort(ip.c_str(), port, &addr6_);
	}
	else
	{
		bzero(&addr_, sizeof(addr_));
		sockets::fromIpPort(ip.c_str(), port, &addr_);
	}
}



std::string InetSocketAddress::toIp() const
{
	char buf[64] = "";
	sockets::toIp(buf, sizeof(buf), toSockAddress());
	return buf;
}

std::string InetSocketAddress::toIpPort() const
{
	char buf[64] = "";
	sockets::toIpPort(buf, sizeof(buf), toSockAddress());
	return buf;
}

uint16_t InetSocketAddress::toPort() const
{
	return sockets::networkToHost16(portNetEndian());
}


uint32_t InetSocketAddress::ipNetEndian() const
{
	assert(family() == AF_INET);
	return addr_.sin_addr.s_addr;
}


bool InetSocketAddress::resolve(const std::string& hostname, InetSocketAddress* out)
{
	char t_resolveBuffer[64 * 1024];
	struct hostent hent;
	struct hostent* he = NULL;
	int herro = 0;
	bzero(&hent, sizeof(hent));
	int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer, sizeof(t_resolveBuffer), &he, &herro);
	if (ret==0&& he!=NULL)
	{
		out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
		return true;
	}
	else
	{
		if (ret!=0)
		{
			std::cout << "InetSocketAddress::resolve failed" << std::endl;
		}
		return false;
	}
}