#include <fcntl.h>
#include "event2/listener.h"
#include "Listener.h"
#include"NioEventLoop.h"
#include"ChannelConfig.h"
#include <iostream>

static const int kDefaultBackLog=128;

Listener::Listener(NioEventLoop* eventLoop, ChannelConfig* config)
	: eventLoop_(eventLoop)
	, config_(config)
	, listening_(false)
	, reuseable_(0)
	, backlog_(kDefaultBackLog)
	, listener_(NULL)
        , newChannel_()
	, devnull_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
    assert(eventLoop_!=NULL);
    assert(config_!=NULL);
    assert(devnull_>=0);
    std::cout<<"Listener created"<<std::endl;
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
    assert(!listening_);
    listening_=true;
    
    unsigned flags=0;
    flags|=LEV_OPT_CLOSE_ON_FREE;
    flags|=LEV_OPT_CLOSE_ON_EXEC;
    flags|=LEV_OPT_DISABLED;
    if(reuseable_!=0)
    {
        flags|=LEV_OPT_REUSEABLE;
    }
    
    const struct sockaddr* address = addr.toSockAddress();
    listener_ = evconnlistener_new_bind(NioEventLoop::Unsafe(eventLoop_).entrieBase(), newChannel, reinterpret_cast<void*>(this), flags, backlog_, address, sizeof(struct sockaddr_in));
    assert(listener_ != NULL);

    config_->bind(evconnlistener_get_fd(listener_));
    evconnlistener_set_error_cb(listener_, exceptionCaught);
    evconnlistener_enable(listener_);

    std::cout << "3@@@@@@@@ listening at address " << addr.toIpPort() << std::endl;
    
}

void Listener::newChannel(struct evconnlistener* listener, evutil_socket_t sockfd, struct sockaddr* address, int socklen, void* privadata)
{
        std::cout<<"4 @@@@@@@@@@@@@@ static Listener::newChannel..."<<std::endl;
	Listener* self = static_cast<Listener*>(privadata);
	self->newChannel_(sockfd, InetSocketAddress(*sockets::sockaddr_in_cast(address)));
}

void Listener::exceptionCaught(struct evconnlistener* listener,void* privdata)
{
	std::cout << "exception caught when accepting" << std::endl;
	if (errno==EMFILE)
	{
		Listener* self = static_cast<Listener*>(privdata);
		close(self->devnull_);
		int listeningFd = static_cast<int>(evconnlistener_get_fd(listener));
		self->devnull_ = accept(listeningFd, NULL, NULL);
		close(self->devnull_);
		self->devnull_ = open("/dev/null", O_RDONLY | O_CLOEXEC);
	}
}