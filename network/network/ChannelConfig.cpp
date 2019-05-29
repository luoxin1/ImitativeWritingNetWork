#include "ChannelConfig.h"
#include <netinet/tcp.h>
#include <algorithm>
#include"Listener.h"
#include"NioSocketChannel.h"


ChannelConfig& ChannelConfig::option(ChannelOption opt, int optval)
{
	if (optval>kIncalidOptval)
	{
		switch (opt)
		{
		case OPT_BACKlOG:
		case OPT_REUSEADDR:
		case OPT_REUSEPORT:
			listenerOps_.push_back(OptionValue(opt, 0, optval));
			break;
		case OPT_READLOWWATERMARK:
		case OPT_READHIGHWATERMARK:
		case OPT_READIDLEKETIMEOUT:
		case OPT_WRITEIDELETIMEOUT:
		case OPT_ALLIDELRTIMEOUT:
			listenerOps_.push_back(OptionValue(opt, 0, optval));
			break;
		case OPT_SOCKETDEBUG:
			listenerOps_.push_back(OptionValue(SO_DEBUG, SOL_SOCKET, optval));
			break;
		case OPT_KEEPALIVE:
			listenerOps_.push_back(OptionValue(SO_KEEPALIVE, SOL_SOCKET, optval));
			break;
		case OPT_DONTROUTE:
			listenerOps_.push_back(OptionValue(SO_DONTROUTE, SOL_SOCKET, optval));
			break;
		case OPT_SOCKETRCVBUF:
			listenerOps_.push_back(OptionValue(SO_RCVBUF, SOL_SOCKET, optval));
			break;
		case OPT_SOCKETSENDBUF:
			listenerOps_.push_back(OptionValue(SO_SNDBUF, SOL_SOCKET, optval));
			break;
		case OPT_TCPNODELAY:
			listenerOps_.push_back(OptionValue(TCP_NODELAY, IPPROTO_TCP, optval));
			break;
		case OPT_TCPMAXSEG:
			listenerOps_.push_back(OptionValue(TCP_MAXSEG, IPPROTO_TCP, optval));
			break;
		default:
			break;
		}
		cacheOps_.insert(std::make_pair(opt, optval));
	}
	return *this;
}

int ChannelConfig::optionValue(ChannelOption opt)
{
	auto it = cacheOps_.find(opt);
	if (it!=cacheOps_.end())
	{
		return it->second;
	}
	return 0;
}

void ChannelConfig::bind(Listener* listener) const
{
	std::for_each(
		listenerOps_.begin(), listenerOps_.end(), [&listener](const OptionValue& opt)
	{
		listener->option(static_cast<ChannelOption>(opt.opt_), opt.optval_);
	});
}

void ChannelConfig::bind(NioSocketChannel* channel) const
{
	std::for_each(
		channelOps_.begin(), channelOps_.end(), [&channel](const OptionValue& opt)
	{
		channel->option(static_cast<ChannelOption>(opt.opt_), opt.optval_);
	});
}

void ChannelConfig::bind(Socket* sock) const
{
	std::for_each(
		nativeSockOps_.begin(), nativeSockOps_.end(), [&sock](const OptionValue& opt)
	{
		sock->option(opt.opt_,opt.level_, opt.optval_);
	});
}

void ChannelConfig::bind(evutil_socket_t sockfd) const
{
	std::for_each(
		nativeSockOps_.begin(), nativeSockOps_.end(), [&sockfd](const OptionValue& opt)
	{
		sockets::setOption(static_cast<int>(sockfd), opt.opt_, opt.level_, opt.optval_);
	});
}