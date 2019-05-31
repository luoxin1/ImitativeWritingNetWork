#ifndef __SERVERBOOTSTRAP_H__
#define __SERVERBOOTSTRAP_H__
#include "boost/noncopyable.hpp"
#include "boost/shared_ptr.hpp"
#include "InetSocketAddress.h"
#include "NioEventLoop.h"
#include "Types.h"
#include "ChannelOption.h"
#include <string>
#include "boost/unordered_map.hpp"
#include "event2/util.h"
#include "boost/atomic.hpp"
#include "ChannelConfig.h"
#include "NioEventLoopGroup.h"
#include<iostream>

class ServerBootstrap: boost::noncopyable
{
public:
	ServerBootstrap(NioEventLoop* baseLoop,
		const std::string& name,
		const InetSocketAddress& listenAddress);

	~ServerBootstrap();
	NioEventLoop* internalLoop() const 
	{
		return baseLoop_;
	}

	const std::string& name() const
	{
		return name_;
	}

	const std::string& hostPort() const 
	{
		return hostPort_;
	}
	
	const InetSocketAddress& listenAddress() const
	{
		return address_;
	}

	ServerBootstrap& group(const boost::shared_ptr<NioEventLoopGroup>& group);

	ServerBootstrap& chanelInitCallback(const ChannelInitCallback& cb)
	{
		initChannel_ = cb;
		return *this;
	}

	ServerBootstrap& chanelInitCallback(ChannelInitCallback&& cb) 
	{
                std::cout<<"111111111111111"<<std::endl;
		initChannel_ = std::move(cb);
		return *this;
	}

	ServerBootstrap& option(ChannelOption opt,bool on);
	ServerBootstrap& option(ChannelOption opt,int optval);

	void start();

private:
	typedef boost::unordered_map<size_t, NioSocketChannelPtr> channelMap;
	
        
        struct ThreadPartitionChannelHolder
	{
		IdlChanelInspectorPtr inspector_;
		channelMap channels_;

		ThreadPartitionChannelHolder()
			:inspector_()
			,channels_()
		{}

		ThreadPartitionChannelHolder(const IdlChanelInspectorPtr& inspector)
			:inspector_(inspector)
			, channels_()
		{}

	};
	typedef std::map<pid_t, ThreadPartitionChannelHolder> ThreadPartitionChannelHolderMap;

	void setupChannelHolderMaps();
	void newChannel(evutil_socket_t sockfd, const InetSocketAddress& remote);
	void newChannelLoop(NioEventLoop* eventLoop, evutil_socket_t socktfd, const InetSocketAddress& remote);
	void removeChannel(const NioSocketChannelPtr& channel);

private:
	NioEventLoop* baseLoop_;
	const std::string name_;
	const std::string hostPort_;
	InetSocketAddress address_;
	ChannelInitCallback initChannel_;

	boost::atomic_bool started_;
	boost::atomic_uint64_t nextChannelId_;
	ThreadPartitionChannelHolderMap hodlers_;
	ChannelConfig config_;
	ListenerPtr lister_;
	boost::shared_ptr<NioEventLoopGroup> group_;
};
#endif
