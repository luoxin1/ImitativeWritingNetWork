#include "Listener.h"



Listener::Listener(NioEventLoop* eventLoop, ChannelConfig* config)
	:newChannel_()
	, eventLoop_(eventLoop)
	, config_(config)
	, listening_(false)
	, reuseable_(0)
	, backlog_()
	, listener_(NULL)
	, newChannel_()
	, devnull_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
}

Listener::~Listener()
{
	::close(devnull_);
	if (listener_!=NULL)
	{
		evconnlistener_free(listener_);
	}
}

Listener& Listener::option(ChannelOption opt, int opval)
{
	switch (opt)
	{
	case OPT_REUSEPORT:
		reuseable_ = opval;
		break;
	case OPT_BACKlOG:
		backlog_ = opval;
		break;
	default:
		break;
	}
	return *this;
}

void Listener::listen(const InetSocketAddress& addr)
{

}


void Listener::newChannel(struct evconnlistener* listener, evutil_socket_t sockfd, struct socketaddr* address, int socklen, void* privadata)
{

}