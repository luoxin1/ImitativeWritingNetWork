#ifndef __NIOSOCKETCHANNEL_H__
#define __NIOSOCKETCHANNEL_H__
#include "boost/any.hpp"
#include "boost/atomic.hpp"
#include "IdlChanelInspector.h"
#include "boost/enable_shared_from_this.hpp"
#include "ChannelPipeline.h"

class NioSocketChannel:
	boost::noncopyable,
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
		context_ = context;
	}

	boost::any* getMutableContext()
	{
		return &context_;
	}

	const boost::any& getContext() const 
	{
		return context_;
	}

	size_t channelId() const 
	{
		return id_;
	}

	const std::string& channelName() const 
	{
		return name_;
	}

	const InetSocketAddress& remote() const 
	{
		return remote_;
	}

	const InetSocketAddress& local() const
	{
		return local_;
	}

	bool isActive() const
	{
		return state_ == kActive;
	}

	NioSocketChannel& option(ChannelOption opt, int optval);

	void write(const void* msg, size_t len);
	void write(const BufferPtr& buf);
	void write(const boost::shared_ptr<std::string>& data);

	void write(const void* msg, size_t len, WritePromiseCallback&& cb);
	void write(const BufferPtr& buf, WritePromiseCallback&& cb);
	void write(const boost::shared_ptr<std::string>& data, WritePromiseCallback&& cb);

	void writeAndFlush(Bytebuf&& buf);
	void writeAndFlush(std::string&& data);
	void writeAndFlush(Builder* buf);
	void writeAndFlush(std::string* data);

	void writeAndFlush(Bytebuf&& buf,WritePromiseCallback&& cb);
	void writeAndFlush(std::string&& data, WritePromiseCallback&& cb);
	void writeAndFlush(Builder* buf, WritePromiseCallback&& cb);
	void writeAndFlush(std::string* data, WritePromiseCallback&& cb);

	void shutdown();
	void close();
	void closeWtihDelay(double seconds);

	void established();
	void destroyed();

	friend class ChannelPipeline;
	friend class ChannelEntry;

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
		state_ = state;
	}

	ChannelState channelState() const
	{
		return state_;
	}

	void writeAndFlushInLoop(Buffer* buf, const WritePromiseCallbackPtr& promise);
	void writeAndFlushInLoop(std::string* data, const WritePromiseCallbackPtr& promise);
	void writeAndFlushInLoop(const Bytebuf& buf, const WritePromiseCallbackPtr& promise);
	void writeAndFlushInLoop(const boost::shared_ptr<std::string>& data, const WritePromiseCallbackPtr& promise);
	
	void shutdownInLoop();
	void closeInLoop();

	void handleClose();
	void handleError(bool invokeWritePromise = false);

	const char* stateToString() const;


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