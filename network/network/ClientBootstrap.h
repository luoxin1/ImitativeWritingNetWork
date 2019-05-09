#ifndef __CLIENTBOOTSTRAP_H__
#define __CLIENTBOOTSTRAP_H__

class ClientBootstrap :boost::noncopyable
{
public:
	ClientBootstrap(NioEventLoop* eventLoop,
		std::string& name,
		const InetSocketAddress& remote);
	~ClientBootstrap();

	NioEventLoop* internalLoop() const { return eventLoop_; }


private:
	void newChannel(evutil_socket_t socketfd);
	void removeChannel(const NioSocketChannelPtr& channel);

private:
	NioEventLoop* eventLoop_;
	std::string name_;

	ChannelConfig config_;
	ConnectorPtr connector_;
	InetSocketAddress remote_;

	boost::atomic_bool retry_;
	size_t nextChannel_;

	mutable boost::mutex mtx_;
	NioSocketChannelPtr channel_;

	IdlChanelInspectorPtr inspector_;
	ChannelInitCallback initChannel_;
};
#endif