#include "ServerBootstrap.h"
#include "NioSocketChannel.h"
#include "Listener.h"
#include "boost/bind.hpp"
#include <iostream>
#include"IdlChanelInspector.h"
#include"ChannelInitailizer.h"

ServerBootstrap::ServerBootstrap(NioEventLoop* baseLoop,
	const std::string& name,
	const InetSocketAddress& listenAddress)
	:baseLoop_(baseLoop)
	, name_(name)
	, hostPort_(listenAddress.toIpPort())
        , started_(false)
	, address_(listenAddress)
        , nextChannelId_(0)
        , hodlers_()
        , config_()
        , lister_(new Listener(baseLoop,&config_))
	, group_()
        , initChannel_(defaultInitChannel)//最开始指向的是这里，后来发生了变化
{
	
}

ServerBootstrap::~ServerBootstrap()
{
	std::for_each(hodlers_.begin(), hodlers_.end(), [](std::pair<const pid_t, ThreadPartitionChannelHolder> &it)
	{
		channelMap& channels = it.second.channels_;
		std::for_each(channels.begin(), channels.end(), [](std::pair<const size_t, NioSocketChannelPtr>& internalIt)
		{
			NioSocketChannelPtr channel(internalIt.second);
			internalIt.second.reset();
			NioEventLoop* eventLoop = channel->internalLoop();
			eventLoop->execute(std::move(boost::bind(&NioSocketChannel::destroyed, channel)));
			channel.reset();
			
		});
	});
		
}


ServerBootstrap& ServerBootstrap::group(const boost::shared_ptr<NioEventLoopGroup>& group)
{
	group_ = group;
	return *this;
}


ServerBootstrap& ServerBootstrap::option(ChannelOption opt, bool on)
{
	bool check = opt == OPT_REUSEPORT || opt == OPT_KEEPALIVE || opt == OPT_TCPNODELAY || opt == OPT_DONTROUTE;
	if (check)
	{
		int optval = on ? 1 : 0;
		config_.option(opt, optval);
	}
	return *this;
}

ServerBootstrap& ServerBootstrap::option(ChannelOption opt, int optval)
{
	bool check = opt == OPT_BACKlOG ||
		opt == OPT_READLOWWATERMARK ||
		opt == OPT_READHIGHWATERMARK ||
		opt == OPT_READIDLEKETIMEOUT ||
		opt == OPT_WRITEIDELETIMEOUT ||
		opt == OPT_ALLIDELRTIMEOUT ||
		opt == OPT_SOCKETRCVBUF ||
		opt == OPT_SOCKETSENDBUF ||
		opt == OPT_TCPMAXSEG;
	if (check)
	{
		config_.option(opt, optval);
	}
	return *this;

}

void ServerBootstrap::start()
{
	assert(!started_);
	started_ = true;
	if (!group_)
	{
		group_.reset(new NioEventLoopGroup(baseLoop_, "internalGroup"));
	}
	if (!group_->started())
	{
		group_->start();
	}

	setupChannelHolderMaps();

	config_.bind(lister_.get());
        
        std::cout<<"1@@@@@@@@@ ServerBootstrap::start()"<<std::endl;
	lister_->newChannelCallback(std::move(boost::bind(&ServerBootstrap::newChannel,this,_1,_2)));
	baseLoop_->execute(std::move(boost::bind(&Listener::listen,get_pointer(lister_),address_)));
}


void ServerBootstrap::setupChannelHolderMaps()
{
	size_t readerIdleSeconds = config_.optionValue(OPT_READIDLEKETIMEOUT);
	size_t writerIdleSeconds = config_.optionValue(OPT_WRITEIDELETIMEOUT);
	size_t allIdleSeconds = config_.optionValue(OPT_ALLIDELRTIMEOUT);

	std::cout << "readerIdleSeconds:" << readerIdleSeconds << " writerIdleSeconds:" << writerIdleSeconds << " allIdleSeconds:"<<allIdleSeconds << std::endl;

	std::vector<NioEventLoop*> loops = group_->allLoop();
	std::for_each(loops.begin(), loops.end(), [&](NioEventLoop*& loop)
	{
		IdlChanelInspectorPtr inspector(new IdlChanelInspector(loop, readerIdleSeconds, writerIdleSeconds, allIdleSeconds));
                hodlers_.insert(std::make_pair(loop->threadId(),ThreadPartitionChannelHolder(inspector)));
	}
	);
}

void ServerBootstrap::newChannel(evutil_socket_t sockfd, const InetSocketAddress& remote)
{
        std::cout<<"5@@@@@@@@ ServerBootstrap::newChannel"<<std::endl;
	NioEventLoop* loop = group_->lightWeighted(NioEventLoop::kAdd);
	loop->execute(std::move(boost::bind(&ServerBootstrap::newChannelLoop,this,loop,sockfd,remote)));
}

void ServerBootstrap::newChannelLoop(NioEventLoop* eventLoop, evutil_socket_t socktfd, const InetSocketAddress& remote)
{
	std::string peer(std::move(remote.toIpPort()));
	std::string local(std::move(address_.toIpPort()));
	char channelName[64] = { 0 };
	size_t channelId = nextChannelId_++;
	evutil_snprintf(channelName, sizeof(channelName),"%s<-->%s_/%s#%lu", peer.c_str(), local.c_str(), name_.c_str(), channelId);

	std::cout << "create channel " << channelName << " in thread " << eventLoop->threadId() << std::endl;

	ThreadPartitionChannelHolder& holder = hodlers_[eventLoop->threadId()];
	NioSocketChannelPtr channel = NioSocketChannel::Builder()
		.eventLoop(eventLoop)
		.id(channelId)
		.name(std::move(std::string(channelName)))
		.sockfd(socktfd)
		.remote(remote)
		.local(address_)
		.inspector(holder.inspector_.get())
		.build();

	config_.bind(channel.get());
	channel->channelCloseCallback(std::move(boost::bind(&ServerBootstrap::removeChannel,this,_1)));
        
        std::cout<<"6@@@@@@@@ ServerBootstrap::newChannelLoop"<<std::endl;
	initChannel_(ChannelInitailizerPtr(new ChannelInitailizer(channel)));

	holder.channels_.insert(std::move(std::make_pair(channelId,channel)));
	channel->established();
}

void ServerBootstrap::removeChannel(const NioSocketChannelPtr& channel)
{
	NioEventLoop* loop = channel->internalLoop();
	channel->destroyed();
	hodlers_[loop->threadId()].channels_.erase(channel->channelId());
	loop->rebalance(NioEventLoop::kDelete);
}