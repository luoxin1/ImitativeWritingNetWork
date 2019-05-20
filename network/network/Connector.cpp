#include "Connector.h"
#include "boost/bind.hpp"
#include <iostream>

static const int kInitRetryDelayMs = 500;
static const int kMaxRetryDelayMs = 30000;

#define BUFFEREVENT_RELEASE(be)\
	do\
	{\
		if()\
		{\
			bufferevent_free(be);\
			be=NULL;\
		}\
	}while(false)

#define BUFFEREVENT_CLOSESOCKET(be)\
	do \
	{\
		if (be!=NULL)\
		{\
			evutil_socket_t sockfd = bufferevent_getfd(be);\
			if (sockfd>=0)\
			{\
				evutil_closesocket(sockfd);\
			}\
		}\
	} while (false);

Connector::Connector(NioEventLoop* eventLoop, ChannelConfig* config, const InetSocketAddress& remote)
	:eventLoop_(eventLoop)
	,config_(config)
	,connect_(false)
	,remote_(remote)
	,retryDelaMs_(kInitRetryDelayMs)
	,state_(KDisconnected)
	,be_(NULL)
	,newChannel_()
{
	assert(eventLoop_ != NULL);
	assert(config_ != NULL);
}

Connector::~Connector()
{

}

void Connector::connect()
{
	connect_ = true;
	eventLoop_->execute(std::move(boost::bind(&Connector::connectInLoop,this)));
}

void Connector::reconnect()
{
	setConnectState(KDisconnected);
	retryDelaMs_ = kInitRetryDelayMs;
	connect();
}

void Connector::disconnect()
{
	connect_ = false;
	eventLoop_->execute(std::move(std::bind(&Connector::disconnect,this)));
}

void Connector::connecting()
{
	assert(state_ == KDisconnected);
	setConnectState(kConnecting);
}

void Connector::retryAfter()
{
	std::cout << "retry connect server after " << retryDelaMs_ << " micro seconds" << std::endl;
	BUFFEREVENT_CLOSESOCKET(be_);
	BUFFEREVENT_RELEASE(be_);
	setConnectState(KDisconnected);
	if (connect_)
	{
		eventLoop_->scheduleOnce(std::move(boost::bind(&Connector::connectInLoop,shared_from_this(),retryDelaMs_/1000)));
		retryDelaMs_ = std::min(retryDelaMs_ * 2, kMaxRetryDelayMs);
	}
}

void Connector::connectInLoop()
{
	be_ = bufferevent_socket_new(NioEventLoop::Unsafe(eventLoop_).entrieBase(), -1, 0);
	assert(be_ != NULL);
	bufferevent_setcb(be_, NULL, NULL, &Connector::eventCaught, reinterpret_cast<void*>(this));
	struct sockaddr* remote =const_cast<struct sockaddr*>(remote_.toSockAddress());
	config_->bind(bufferevent_getfd(be_));
	bufferevent_socket_connect(be_, remote_, sizeof(struct sockaddr_in));
}
	
void Connector::disconnectInLoop()
{
	BUFFEREVENT_CLOSESOCKET(be_);
	BUFFEREVENT_RELEASE(be_);
	setConnectState(KDisconnected);
}

void Connector::handleError()
{
	switch (errno)
	{
	case EINTR:
	case EISCONN:
	case EINPROGRESS:
		connecting();
		break;
	case EAGAIN:
	case EADDRINUSE:
	case EADDRNOTAVAIL:
	case ECONNREFUSED:
	case ENETUNREACH:
		retryAfter();
		break;
	case EACCES:
	case EPERM:
	case EALREADY:
	case EBADF:
	case EFAULT:
	case EAFNOSUPPORT:
		BUFFEREVENT_CLOSESOCKET(be_);
		BUFFEREVENT_RELEASE(be_);
		break;
	default:
		BUFFEREVENT_CLOSESOCKET(be_);
		BUFFEREVENT_RELEASE(be_);
		break;
	}
}

void Connector::eventCaught(struct bufferevent* be, short what, void* privdata)
{
	Connector* self = static_cast<Connector*>(privdata);
	if (what & BEV_EVENT_ERROR)
	{
		self->handleError();
	}
	else if (what & BEV_EVENT_CONNECTED)
	{
		evutil_socket_t sockfd = bufferevent_getfd(self->be_);
		evutil_make_socket_closeonexec(sockfd);
		BUFFEREVENT_RELEASE(be_);
		if (self->newChannel_)
		{
			self->newChannel_(sockfd);
		}

	}
}
