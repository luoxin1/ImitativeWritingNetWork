#include "NioSocketChannel.h"


NioSocketChannel::NioSocketChannel(NioEventLoop* eventLoo,
	size_t id,
	std::string&& name,
	evutil_socket_t sockfd,
	InetSocketAddress remote,
	InetSocketAddress local,
	IdlChanelInspector* inspector)
	:eventLoop_(eventLoo)
	,id_(id)
	,name_(std::move(name))
	,sockfd_(sockfd)
	,pipeline_(new ChannelPipeline(eventLoo,sockfd,0))
	,remote_(remote)
	,local_(local)
	,inspector_(inspector)
	,context_()
{
	assert(eventLoop_ != NULL);
}

NioSocketChannel::~NioSocketChannel()
{
	assert(state_ == kInactive);
	std::count << "channel " << name_ << " destroy" << std::endl;
}

NioSocketChannel& NioSocketChannel::channelActiveCallback(const ChannelActiveCallback& cb)
{
	pipeline_->channelActiveCallback(cb);
	return *this;
}

NioSocketChannel& NioSocketChannel::channelInActiveCallback(const ChannelInActiveCallback& cb)
{
	pipeline_->channelInActiveCallback(cb);
	return *this;
}

NioSocketChannel& NioSocketChannel::messageCallback(const MessageCallback& cb)
{
	pipeline_->messageCallback(cb);
	return *this;
}

NioSocketChannel& NioSocketChannel::idleStateCallback(const IdleStateCallback& cb)
{
	pipeline_->idleStateCallback(cb);
	return *this;
}

NioSocketChannel& NioSocketChannel::channelCloseCallback(const ChannelCloseCallback& cb)
{
	pipeline_->channelCloseCallback(cb);
	return *this;
}

NioSocketChannel& NioSocketChannel::channelActiveCallback(const ChannelActiveCallback&& cb)
{
	pipeline_->channelActiveCallback(std::move(cb));
	return *this;
}

NioSocketChannel& NioSocketChannel::channelInActiveCallback(const ChannelInActiveCallback&& cb)
{
	pipeline_->channelInActiveCallback(std::move(cb));
	return *this;
}

NioSocketChannel& NioSocketChannel::messageCallback(const MessageCallback&& cb)
{
	pipeline_->messageCallback(std::move(cb));
	return *this;
}

NioSocketChannel& NioSocketChannel::idleStateCallback(const IdleStateCallback&& cb)
{
	pipeline_->idleStateCallback(std::move(cb));
	return *this;
}

NioSocketChannel& NioSocketChannel::channelCloseCallback(const ChannelCloseCallback&& cb)
{
	pipeline_->channelCloseCallback(std::move(cb));
	return *this;
}

NioSocketChannel& NioSocketChannel::option(ChannelOption opt, int optval)
{
	switch (optval)
	{
	case OPT_READLOWWATERMARK:
	case OPT_READHIGHWATERMARK:
	case OPT_READIDLEKETIMEOUT:
	case OPT_WRITEIDELETIMEOUT:
	case OPT_ALLIDELRTIMEOUT:
		pipeline_->option(opt, optval);
		break;
	default:
		break;
	}

	return *this;
}

void NioSocketChannel::write(const void* msg, size_t len)
{
	if (state_==kActive)
	{
		Buffer* safe = new Buffer();
		safe->append(msg, len);
		if (eventLoop_->inEventLoop())
		{
			writeAndFlushInLoop(safe, WritePromiseCallbackPtr());
		}
		else
		{
			eventLoop_->execute(std::move(boost::bind<void,NioSocketChannel,Buffer*,const WritePromiseCallbackPtr&>(&NioSocketChannel::writeAndFlushInLoop,this,safe,WritePromiseCallbackPtr())));
		}
	}
}

void NioSocketChannel::write(const BufferPtr& buf)
{
	if (state_==kActive)
	{
		if (eventLoop_->inEventLoop())
		{
			writeAndFlushInLoop(buf, WritePromiseCallbackPtr());
		}
		else
		{
			eventLoop_->execute(std::move(boost::bind<void,NioSocketChannel,const BufferPtr&,const WritePromiseCallbackPtr>(&NioSocketChannel::writeAndFlushInLoop,this,buf,WritePromiseCallbackPtr())));
		}
	}
}

void NioSocketChannel::write(const boost::shared_ptr<std::string>& data)
{
	if (state_==kActive)
	{
		if (eventLoop_->inEventLoop())
		{
			writeAndFlushInLoop(data, WritePromiseCallbackPtr());
		}
		else
		{
			eventLoop_->execute(std::move(boost::bind<void,NioSocketChannel,const boost::shared_ptr<std::string>&,const WritePromiseCallbackPtr&>(&NioSocketChannel::writeAndFlushInLoop, this, data, WritePromiseCallbackPtr())))
		}
	}
}

void NioSocketChannel::write(const void* msg, size_t len, WritePromiseCallback&& cb)
{

}

void NioSocketChannel::write(const BufferPtr& buf, WritePromiseCallback&& cb)
{

}
	
void NioSocketChannel::write(const boost::shared_ptr<std::string>& data, WritePromiseCallback&& cb)
{

}

void NioSocketChannel::writeAndFlush(Bytebuf&& buf)
{

}
	
void NioSocketChannel::writeAndFlush(std::string&& data)
{

}
	
void NioSocketChannel::writeAndFlush(Builder* buf)
{

}
	
void NioSocketChannel::writeAndFlush(std::string* data)
{

}

void NioSocketChannel::writeAndFlush(Bytebuf&& buf, WritePromiseCallback&& cb)
{

}
	
void NioSocketChannel::writeAndFlush(std::string&& data, WritePromiseCallback&& cb) 
{

}

void NioSocketChannel::writeAndFlush(Builder* buf, WritePromiseCallback&& cb)
{

}
	
void NioSocketChannel::writeAndFlush(std::string* data, WritePromiseCallback&& cb)
{

}

void NioSocketChannel::shutdown()
{

}

void NioSocketChannel::close()
{

}

void NioSocketChannel::closeWtihDelay(double seconds)
{

}

void NioSocketChannel::established()
{

	}
void NioSocketChannel::destroyed()
{

}

void NioSocketChannel::writeAndFlushInLoop(Buffer* buf, const WritePromiseCallbackPtr& promise)
{

}
	
void NioSocketChannel::writeAndFlushInLoop(std::string* data, const WritePromiseCallbackPtr& promise)
{

}

void NioSocketChannel::writeAndFlushInLoop(const Bytebuf& buf, const WritePromiseCallbackPtr& promise)
{

}

void NioSocketChannel::writeAndFlushInLoop(const boost::shared_ptr<std::string>& data, const WritePromiseCallbackPtr& promise)
{

}

void NioSocketChannel::shutdownInLoop()
{

}
	
void NioSocketChannel::closeInLoop()
{

}

void NioSocketChannel::handleClose()
{

}
	
void NioSocketChannel::handleError(bool invokeWritePromise)
{

}

const char* NioSocketChannel::stateToString() const
{

}
