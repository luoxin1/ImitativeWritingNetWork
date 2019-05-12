#ifndef __CLIENTBOOTSTRAP_H__
#define __CLIENTBOOTSTRAP_H__
#include "boost/noncopyable.hpp"
#include "NioEventLoop.h"
#include "InetSocketAddress.h"
#include "Connector.h"
#include "IdlChanelInspector.h"

class ClientBootstrap :boost::noncopyable
{
public:
	ClientBootstrap(NioEventLoop* eventLoop,
		std::string& name,
		const InetSocketAddress& remote);
	~ClientBootstrap();

	NioEventLoop* internalLoop() const { return eventLoop_; }

	ClientBootstrap& enableRetry(bool on)
	{
		retry_ = on;
		return *this;
	}

	ClientBootstrap& channelInitCallback(const ChannelInitCallback& cb)
	{
		initChannel_ = cb;
		return *this;
	}

	ClientBootstrap& channelInitCallback(const ChannelInitCallback&& cb)
	{
		initChannel_ = std::move(cb);
		return *this;
	}

	ClientBootstrap& option(ChannelOption opt, bool on);
	ClientBootstrap& option(ChannelOption opt, int optvalue);

	NioSocketChannelPtr channel() const
	{
		boost::lock_guard<boost::mutex> lock(mtx_);
		return channel_;
	}

	void connect();
	void disconnect();
private:
	void newChannel(evutil_socket_t socketfd);
	void removeChannel(const NioSocketChannelPtr& channel);

private:
	NioEventLoop* eventLoop_;
	std::string name_;

	ChannelConfig config_;
	ConnectorPtr connector_;
	InetSocketAddress remote_;

	boost::atomic_bool retry_;
	size_t nextChannelId_;

	mutable boost::mutex mtx_;
	NioSocketChannelPtr channel_;

	IdlChanelInspectorPtr inspector_;
	ChannelInitCallback initChannel_;
};
#endif