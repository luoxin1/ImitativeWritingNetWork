#ifndef __INETSOCKETADDRESS_H__
#define __INETSOCKETADDRESS_H__
#include<string>

class InetSocketAddress
{
public:
	explicit InetSocketAddress(uint16_t port = 0, bool loopbackOnly = false, bool ipv6 = false);
	InetSocketAddress(const std::string& ip, uint16_t port, bool ipv6 = false);

	explicit InetSocketAddress(const struct socketaddr_in& addr)
		:addr_(addr)
	{

	}

	explicit InetSocketAddress(const struct socketaddr_in6& addr6)
		:addr6_(addr6)
	{

	}

	sa_family_t family() const 
	{
		return addr_.sin_family;
	}
	std::string toIp() const;
	std::string toIpPort() const;
	uint16_t toPort() const;
	
	const struct sockaddr* toSockAddress() const
	{
		return sockets::sockaddr_cast(&addr6_);
	}

	void setSocketAddressInet6(const struct sockaddr_in6& addr6)
	{
		addr6_ = addr6;
	}

	uint32_t ipNetEndian() const;
	uint16_t portNetEndian() const
	{
		return addr_.sin_port;
	}

	static bool resolve(const std::string& hostname, InetSocketAddress* out);

private:
	struct socketaddr_in addr_;
	struct socketaddr_in6 addr6_;
};
#endif
