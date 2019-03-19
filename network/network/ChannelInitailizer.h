#ifndef __CHANNELINITAILIZER_H__
#define __CHANNELINITAILIZER_H__
#include "boost/noncopyable.hpp"
#include "NioSocketChannel.h"
#include "Types.h"

class ChannelInitailizer: private  boost::noncopyable
{
public:
	ChannelInitailizer(const NioSocketChannelPtr& ownerChannel)
		:ownerChannel_(ownerChannel)
	{

	}

	~ChannelInitailizer() {}

	const NioSocketChannelPtr channel() const
	{
		return ownerChannel_;
	}

	void channelActiveCallback(const ChannelActiveCallback& cb) const
	{
		//实现回调
	}

	void channelInactiveCallback(const ChannelInActiveCallback& cb) const
	{

	}

	void messageCallback(const )

private:
	NioSocketChannelPtr ownerChannel_;
};

#endif // !