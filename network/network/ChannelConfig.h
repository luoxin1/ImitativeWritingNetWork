#ifndef __CHANNELCONFIG_H__
#define __CHANNELCONFIG_H__
#include "ChannelOption.h"
#include <map>
#include<vector>
#include"Socket.h"

class NioSocketChannel;
class Listener;

class ChannelConfig
{
public:
	ChannelConfig()
        :listenerOps_()
        ,channelOps_()
        ,nativeSockOps_()
	{

	}

	~ChannelConfig()
	{

	}

	ChannelConfig& option(ChannelOption opt, int optval);
	int optionValue(ChannelOption opt);

	void bind(Listener* listener) const;
	void bind(NioSocketChannel* channel) const;
	void bind(Socket* sock) const;
	void bind(evutil_socket_t sockfd) const;

private:
	struct OpetionValue
	{
		int opt_;
		int level_;
		int optval_;

		OpetionValue()
			:opt_(0)
			,level_(0)
			,optval_(0)
		{

		}

		OpetionValue(int opt, int level, int optval)
			:opt_(opt)
			, level_(level)
			, optval_(optval)
		{

		}
	};
        
        typedef std::vector<OpetionValue> ChannelOptionList;

	ChannelOptionList listenerOps_;
	ChannelOptionList channelOps_;
	ChannelOptionList nativeSockOps_;

	std::map<ChannelOption, int> cacheOps_;
};
#endif
