#ifndef __NIOSOCKETCHANNEL_H__
#define __NIOSOCKETCHANNEL_H__
#include "boost/any.hpp"
#include "boost/atomic.hpp"

class NioSocketChannel
{
public:
	NioSocketChannel();
	~NioSocketChannel();
};

typedef boost::shared_ptr<NioSocketChannel> NioSocketChannelPtr;
void defaultInitChannel(const NioSocketChannelPtr& channelInitializer);

#endif