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
		ownerChannel_->channelActiveCallback(cb);
	}

	void channelInactiveCallback(const ChannelInActiveCallback& cb) const
	{
		ownerChannel_->channelInActiveCallback(cb);
	}

	void messageCallback(const MessageCallback& cb) const
	{
		ownerChannel_->messageCallback(cb);
	}

	void idleStateCallback(const IdleStateCallback& cb) const
	{
		ownerChannel_->idleStateCallback(cb);
	}

	void channelActiveCallback(ChannelActiveCallback&& cb) const
	{
		ownerChannel_->channelActiveCallback(std::move(cb));
	}

	void channelInactiveCallback(ChannelInActiveCallback&& cb) const
	{
		ownerChannel_->channelInActiveCallback(std::move(cb));
	}

	void messageCallback(MessageCallback&& cb) const
	{
		ownerChannel_->messageCallback(std::move(cb));
	}

	void idleStateCallback(IdleStateCallback&& cb) const
	{
		ownerChannel_->idleStateCallback(std::move(cb));
	}

private:
	NioSocketChannelPtr ownerChannel_;
};

typedef boost::shared_ptr< ChannelInitailizer> ChannelInitailizerPtr;

#endif // !