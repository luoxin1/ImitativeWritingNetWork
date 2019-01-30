#include "ServerBootstrap.h"


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

}


ServerBootstrap& ServerBootstrap::group(const boost::shared_ptr<NioEventLoop>& group)
{

}


ServerBootstrap& ServerBootstrap::option(ChannelOption opt, bool on)
{

}

ServerBootstrap& ServerBootstrap::option(ChannelOption opt, int optval)
{

}

void ServerBootstrap::start()
{

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