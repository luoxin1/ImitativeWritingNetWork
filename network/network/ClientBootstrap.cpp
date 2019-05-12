#include "ClientBootstrap.h"
#include "ChannelOption.h"

void safeRemoveChannel(NioEventLoop* eventLoop,const NioSocketChannelPtr& channel)
{
	eventLoop->rebalance(NioEventLoop::kDelete);
	eventLoop->execute(std::move(boost::bind(&NioSocketChannel::destroyed,channel)));
}

void setChannelCloseCallback(const NioSocketChannelPtr channel,const ChannelCloseCallback& cb)
{
	channel->channelCloseCallback(cb);
}

ClientBootstrap::ClientBootstrap(NioEventLoop* eventLoop,
	std::string& name,
	const InetSocketAddress& remote)
	:eventLoop_(eventLoop)
	,name_(std::move(name))
	,config_()
	,connector_(new Connector(eventLoop,&config_,remote))
	,remote_(remote)
	,retry_(false)
	,nextChannelId_(0)
	,mtx_()
	,channel_()
	,inspector_()
	,initChannel_(defaultInitChannel)
{
	connector_->newChannelCallback(std::move(boost::bind(&ClientBootstrap::newChannel, this, _1)));
}

ClientBootstrap::~ClientBootstrap()
{
	std::cout << "ClientBoostrap destroy...." << std::endl;
	NioSocketChannelPtr channel;
	bool unique = false;
	{
		boost::lock_guard<boost::mutex> lock(mtx_);
		unique = channel.unique();
		channel = channel_;
	}
	if (channel)
	{
		assert(eventLoop_ == channel->internalLoop());
		ChannelInitCallback cb = std::move(boost::bind(&safeRemoveChannel,eventLoop_,_1));
		eventLoop_->execute(std::move(boost::bind(&setChannelCloseCallback,channel,cb)));
		if (unique)
		{
			channel->close();
		}
	}
	else
	{
		connector_->disconnect();
	}
}


ClientBootstrap& ClientBootstrap::option(ChannelOption opt, bool on)
{
	bool check = (opt == OPT_KEEPALIVE ||
		opt == OPT_TCPNODELAY ||
		opt == OPT_DONTROUTE);
	if (check)
	{
		int optval = on ? 1 : 0;
		config_.option(opt, optval);
	}
	return *this;
}

ClientBootstrap& ClientBootstrap::option(ChannelOption opt, int optvalue)
{
	bool check = opt == OPT_SOCKETRCVBUF ||
		opt == OPT_SOCKETSENDBUF ||
		opt == OPT_TCPMAXSEG ||
		opt == OPT_READLOWWATERMARK ||
		opt == OPT_READHIGHWATERMARK ||
		opt == OPT_READIDLEKETIMEOUT ||
		opt == OPT_WRITEIDELETIMEOUT ||
		opt == OPT_ALLIDELRTIMEOUT;
	if (check)
	{
		config_.option(opt, optvalue);
	}
	return *this;
}

void ClientBootstrap::connect()
{
	size_t readerIdleSeconds = config_.optionValue(OPT_READIDLEKETIMEOUT);
	size_t writerIdleSeconds = config_.optionValue(OPT_WRITEIDELETIMEOUT);
	size_t readerall= config_.optionValue(OPT_ALLIDELRTIMEOUT);
	inspector_->reset(new IdlChanelInspector(eventLoop_, readerIdleSeconds, writerIdleSeconds, readerIdleSeconds));


	connector_->connect();
}

void ClientBootstrap::disconnect()
{
	boost::lock_guard<boost::mutex> lock(mtx_);
	if (channel_)
	{
		channel_->shutdown();
	}

}

void ClientBootstrap::newChannel(evutil_socket_t socketfd)
{
	eventLoop_->rebalance(NioEventLoop::kAdd);
	InetSocketAddress local(sockets::getLocalAddress(socketfd));

	std::string remote(std::move(remote_.toIpPort()));
	std::string localHost(std::move(local.toIpPort()));

	char connName[64];
	evutil_snprintf(connName, sizeof(connName), "%s#%lu/%s<-->%s", name_.c_str(), nextChannelId_, localHost.c_str(), remote.c_str());

	std::cout << "create channel " << connName << " in thread " << eventLoop_->threadId() << std::endl;

	NioSocketChannelPtr channel = NioSocketChannel::Builder()
		.eventLoop(eventLoop_)
		.id(nextChannelId_++)
		.name(std::move(std::string(connName)))
		.sockfd(socketfd)
		.remote(remote_)
		.local(local)
		.inspector(inspector_.get())
		.build();

	config_.bind(channel.get());
	channel->channelCloseCallback(std::move(boost::bind(&ClientBootstrap::removeChannel, this, _1)));
	initChannel_(ChannelInitailizerPtr(new ChannelInitailizer(channel)));
	{
		boost::lock_guard<boost::mutex> lock(mtx_);
		channel_ = channel;
	}
	channel->established();
}

void ClientBootstrap::removeChannel(const NioSocketChannelPtr& channel)
{
	assert(eventLoop_ == channel->internalLoop());
	eventLoop_->rebalance(NioEventLoop::kDelete);
	channel->destroyed();
	{
		boost::lock_guard<boost::mutex> lock(mtx_);
		channel.reset();
	}
	if (retry_)
	{
		connector_->reconnect();
	}
}