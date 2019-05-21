#ifndef __CHANNELPIPELINE_H__
#define __CHANNELPIPELINE_H__
#include"boost/noncopyable.hpp"
#include"boost/enable_shared_from_this.hpp"
#include"event2/event.h"
#include"ChannelOption.h"
#include"ByteBuf.h"
#include<map>
#include<deque>
#include"Types.h"
#include"NioEventLoop.h"
#include"IdlChanelInspector.h"

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

	ChannelPipeline& channelCloseCallback(const ChannelCloseCallback& cb)
	{
		channelClose_ = cb;
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

	ChannelPipeline& channelCloseCallback(const ChannelCloseCallback&& cb)
	{
		channelClose_ = std::move(cb);
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
	void exceptionCaught(const NioSocketChannelPtr& channel, bool invokeWritePromiss);

	Bytebuf* input() { return &input_; }
	Bytebuf* output() { return &output_; }

	void pushPromise(const WritePromiseCallbackPtr& writePromise)
	{
		writePromis_.push_back(writePromise);
	}


private:
	static void messageReceived(struct bufferevent* be, void* privdata);
	static void writePromise(struct bufferevent* be, void* privdata);
	static void eventCaught(struct bufferevent* be, short what, void* privdata);

	void updateIdleChannelEntry(const NioSocketChannelPtr& selfChannel, IdleState ildestate);
	
private:
	typedef std::pair<int, int> OptionPair;
	Bytebuf input_;
	Bytebuf output_;

	ChannelActiveCallback channelActive_;
	ChannelInActiveCallback channelInActive_;
	MessageCallback messageReceived_;
	IdleStateCallback idleState_;
	ChannelCloseCallback channelClose_;

	NioEventLoop* eventLoop_;
	struct bufferevent* underlying_;

	OptionPair readWaterMark_;
	OptionPair idleStateTimeouts_;

	std::deque<WritePromiseCallbackPtr> writePromis_;

	boost::weak_ptr<ChannelPipeline> self_;
	boost::weak_ptr<NioSocketChannel> selfChannel_;

	std::map<IdleState, WeakChannelEntryPtr> interestIdles_;

};


#endif // !__ChannelPipeline_h__
