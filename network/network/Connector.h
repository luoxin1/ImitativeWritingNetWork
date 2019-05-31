#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__
#include <boost/noncopyable.hpp>
#include "boost/enable_shared_from_this.hpp"
#include "boost/function.hpp"
#include "NioEventLoop.h"
#include "InetSocketAddress.h"
#include "event2/util.h"
#include<event.h>

class ChannelConfig;
class Connector:boost::noncopyable
	,public boost::enable_shared_from_this<Connector>
{
public:
	typedef boost::function<void(evutil_socket_t sockfd)> NewChannelCallback;

	Connector(NioEventLoop* eventLoop, ChannelConfig* config, const InetSocketAddress& remote);
	~Connector();

	void connect();
	void reconnect();
	void disconnect();

	Connector& newChannelCallback(const NewChannelCallback& cb)
	{
		newChannel_ = cb;
		return *this;
	}

	Connector& newChannelCallback(NewChannelCallback&& cb)
	{
		newChannel_ = std::move(cb);
		return *this;
	}

	const InetSocketAddress& remote() const
	{
		return remote_;
	}

private:
	enum ConnectState
	{
		KDisconnected,
		kConnecting,
		kConnected
	};

	void setConnectState(ConnectState state)
	{
		state_ = state;
	}

	void connecting();
	void retryAfter();

	void connectInLoop();
	void disconnectInLoop();

	void handleError();

	static void eventCaught(struct bufferevent* be, short what, void* privdata);

private:

	NioEventLoop* eventLoop_;
	ChannelConfig* config_;

	boost::atomic_bool connect_;
	InetSocketAddress remote_;

	int retryDelaMs_;

	ConnectState state_;
	struct bufferevent* be_;
	NewChannelCallback newChannel_;

};

typedef boost::shared_ptr<Connector> ConnectorPtr;

#endif