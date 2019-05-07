#include "ServerBootstrap.h"
#include "NioSocketChannel.h"
#include "Listener.h"
#include "boost/bind.hpp"

ServerBootstrap::ServerBootstrap(NioEventLoop* baseLoop,
	const std::string& name,
	const InetSocketAddress& listenAddress)
	:baseLoop_(baseLoop)
	, name_(name)
	, hostPort_(listenAddress.toIpPort())
	, address_(listenAddress)
	, initChannel_(defaultInitChannel)
	, started_(false)
	, nextChannelId_(0)
	, hodlers_()
	, config_()
	, lister_(new Listener(baseLoop,&config_))
	, group_()
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


ServerBootstrap& ServerBootstrap::group(const boost::shared_ptr<NioEventLoop>& group)
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
		group_.reset(new NioEventLoop(baseLoop_, "internalGroup"));
	}
	if (!group_->started())
	{
		group_->start();
	}

	setupChannelHolderMaps();

	config_.bind(lister_.get());
	lister_->newChannelCallback(std::move(boost::bind(&ServerBootstrap::newChannelLoop,this,_1,_2)));
	baseLoop_->execute(std::move(boost::bind(&Listener::listen,get_pointer(lister_),address_)));
}


void ServerBootstrap::setupChannelHolderMaps()
{

}

void ServerBootstrap::newChannelLoop(NioEventLoop* eventLoop, evutil_socket_t socktfd, const InetSocketAddress& remote)
{

}

void ServerBootstrap::removeChannel(const NioSocketChannelPtr& channel)
{

}