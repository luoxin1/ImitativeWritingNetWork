#ifndef __CHANNELPIPELINE_H__
#define __CHANNELPIPELINE_H__
#include"boost/noncopyable.hpp"
#include"boost/enable_shared_from_this.hpp"
#include"NioEventLoop.h"
#include"event2/event.h"
#include"Types.h"
#include"ChannelOption.h"
#include"ByteBuf.h"

class ChannelPipeline:boost::noncopyable
	,public boost::enable_shared_from_this<ChannelPipeline>
{
public:
	ChannelPipeline(NioEventLoop* eventLoop, evutil_socket_t sockfd, int flag);
	~ChannelPipeline();

	ChannelPipeline& channelActiveCallback(const ChannelActiveCallback& cb)
	{
		channelActive_ = cb;
		return *this;
	}

	ChannelPipeline& channelInActiveCallback(const ChannelInActiveCallback& cb)
	{
		channelInActive_ = cb;
		return *this;
	}

	ChannelPipeline& messageCallback(const MessageCallback& cb)
	{
		messageReceived_ = cb;
		return *this;
	}

	ChannelPipeline& idleStateCallback(const IdleStateCallback& cb)
	{
		idleState_ = cb;
		return *this;
	}



	ChannelPipeline& channelActiveCallback(const ChannelActiveCallback&& cb)
	{
		channelActive_ = std::move(cb);
		return *this;
	}

	ChannelPipeline& channelInActiveCallback(const ChannelInActiveCallback&& cb)
	{
		channelInActive_ = std::move(cb);
		return *this;
	}

	ChannelPipeline& messageCallback(const MessageCallback&& cb)
	{
		messageReceived_ = std::move(cb);
		return *this;
	}

	ChannelPipeline& idleStateCallback(const IdleStateCallback&& cb)
	{
		idleState_ = std::move(cb);
		return *this;
	}

	ChannelPipeline& option(ChannelOption opt, int optval);

	void tie(const NioSocketChannelPtr& selfChannel);

	void enableRead();
	void enableWrite();
	void enableAll();
	void disableRead();
	void disableWrite();
	void disableAll();

	bool isReading();
	bool isWritting();
	bool isNonEvent();

	void channelActive(const NioSocketChannelPtr& channel);
	void channelInActive(const NioSocketChannelPtr& channel);
	void idleStateTriggered(const NioSocketChannelPtr& channel, IdleState idleState);

	void close(const NioSocketChannelPtr& channel);
	void exceptionCaught(const NioSocketChannel& channel, bool invokeWritePromiss);

	Bytebuf* input() { return &input_; }
	Bytebuf* output() { return &output_; }

	void pushPromise(const WritePromiseCallbackPtr& writePromise)
	{

	}


private:
	Bytebuf input_;
	Bytebuf output_;

private:


	ChannelActiveCallback channelActive_;
	ChannelInActiveCallback channelInActive_;
	MessageCallback messageReceived_;
	IdleStateCallback idleState_;
	ChannelCloseCallback channelClose_;
};

typedef boost::shared_ptr<ChannelPipeline> ChannelPipelinePtr;

#endif // !__ChannelPipeline_h__
