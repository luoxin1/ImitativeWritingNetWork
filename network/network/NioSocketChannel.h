#ifndef __NIOSOCKETCHANNEL_H__
#define __NIOSOCKETCHANNEL_H__
#include "boost/any.hpp"
#include "boost/atomic.hpp"
#include "IdlChanelInspector.h"

class NioSocketChannel:boost::noncopyable,
	public boost::enable_shared_from_this<NioSocketChannel>
{
public:
	NioSocketChannel();
	~NioSocketChannel();

public:
	class Builder
	{
	public:
		Builder()
			:eventLoop_(NULL)
			, id_(0)
			, name_()
			, sockfd_(-1)
			, remote_()
			, local_()
		{

		}

		~Builder()
		{

		}

		Builder& eventLoop(NioEventLoop* eventLoop)
		{
			eventLoop_ = eventLoop;
			return *this;
		}

		Builder& id(size_t id)
		{
			id_ = id;
			return *this;
		}
	
		Builder& name(std::string&& name)
		{
			name_ = std::move(name);
			return *this;
		}

		Builder& sockfd(evutil_socket_t sockfd)
		{
			sockfd_ = sockfd;
			return *this;
		}

		Builder& remote(InetSocketAddress remote)
		{
			remote_ = remote;
			return *this;
		}

		Builder& local(InetSocketAddress local)
		{
			local_ = local;
			return *this;
		}

		Builder& inspector(IdlChanelInspector* inspector)
		{
			inspector_ = inspector;
			return *this;
		}

		NioSocketChannelPtr build();

	private:
		NioEventLoop* eventLoop_;
		size_t id_;
		std::string name_;
		evutil_socket_t sockfd_;
		InetSocketAddress remote_;
		InetSocketAddress local_;
		IdlChanelInspector inspector_;
	};

	~NioSocketChannel();

	NioSocketChannel& channelActiveCallback(const ChannelActiveCallback& cb);
	NioSocketChannel& channelInActiveCallback(const ChannelInActiveCallback& cb);
	NioSocketChannel& messageCallback(const MessageCallback& cb);
	NioSocketChannel& idleStateCallback(const IdleStateCallback& cb);
	NioSocketChannel& channelClostCallback(const ChannelCloseCallback& cb);

	NioSocketChannel& channelActiveCallback(const ChannelActiveCallback&& cb);
	NioSocketChannel& channelInActiveCallback(const ChannelInActiveCallback&& cb);
	NioSocketChannel& messageCallback(const MessageCallback&& cb);
	NioSocketChannel& idleStateCallback(const IdleStateCallback&& cb);
	NioSocketChannel& channelClostCallback(const ChannelCloseCallback&& cb);

	NioEventLoop* internalLoop() const
	{
		return eventLoop_;
	}

	void setContext(const boost::any& context)
	{
		
	}


private:
	enum ChannelState
	{
		kInactive,
		kActiving,
		kActive,
		kInactiving
	};

	NioSocketChannel()
	{

	}

	void setChannelState(ChannelState state)
	{

	}

private:
	NioEventLoop* eventLoop_;
	const size_t id_;
	const std::string name_;

	Socket socket_;
	ChannelState state_;

	ChannelPipelinePtr pipeline_;
	const InetSocketAddress remote_;
	const InetSocketAddress local_;
	IdlChanelInspector* inspector_;

	boost::any context_;
};

typedef boost::shared_ptr<NioSocketChannel> NioSocketChannelPtr;
void defaultInitChannel(const NioSocketChannelPtr& channelInitializer);

#endif