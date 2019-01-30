#ifndef __LISTENER_H__
#define __LISTENER_H__
#include "boost/noncopyable.hpp"
#include "NioEventLoop.h"
#include "ChannelConfig.h"
#include "boost/function.hpp"
#include "event2/util.h"
#include "boost/atomic.hpp"
#include "InetSocketAddress.h"
#include "ChannelOption.h"

class Listener:boost::noncopyable
{
public:
	typedef boost::function<void(evutil_socket_t sockfd, const InetSocketAddress& remote)> NewChannelCallback;

	Listener(NioEventLoop* eventLoop, ChannelConfig* config);
	~Listener();

	Listener& newChannelCallback(const NewChannelCallback& cb)
	{
		newChannel_ = cb;
		return *this;
	}

	Listener& newChannelCallback(const NewChannelCallback&& cb)
	{
		newChannel_ = std::move(cb);
		return *this;
	}

	Listener& option(ChannelOption opt, int opval);
	void listen(const InetSocketAddress& addr);

private:
	typedef std::vector<std::pair<int, int>> OptionList;
	NewChannelCallback newChannel_;
	NioEventLoop* eventLoop_;
	ChannelConfig* config_;
	boost::atomic_bool listening_;
	int reuseable_;
	int backlog_;
	struct evconnlistener* listener_;
	int devnull_;
private:
	static void newChannel(struct evconnlistener* listener, evutil_socket_t sockfd, struct socketaddr* address, int socklen, void* privadata);
};
typedef boost::shared_ptr<Listener> ListenerPtr;
#endif
