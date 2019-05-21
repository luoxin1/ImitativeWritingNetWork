#include "NioSocketChannel.h"


void defaultChannelActive(const NioSocketChannelPtr& channel)
{
	std::cout << "channel " << channel->channelName() << " is Active" << std::endl;;
}

void defaultChannelInActive(const NioSocketChannelPtr& channel)
{
	std::cout << "channel " << channel->channelName() << " is InActive" << std::endl;;
}

void defaultMessageReceived(const NioSocketChannelPtr& channel,Bytebuf& input,Timestamp receiveTime)
{
	std::cout << "defaultMessageReceived " << input.readableBytes()<<" was read and discard all" << std::endl;
	input.discardBytes(input.readableBytes());
}

void defaultState(const NioSocketChannelPtr& channel,IdleState idleStae)
{
	std::cout << "defaultState channel " << channel->channelName()  << std::endl;
	channel->close();
}

void defaultInitChannel(const ChannelInitailizerPtr& channelInitailizer)
{
	channelInitailizer->channelActiveCallback(defaultChannelActive);
	channelInitailizer->channelActiveCallback(defaultChannelInActive);
	channelInitailizer->channelActiveCallback(defaultMessageReceived);
	channelInitailizer->channelActiveCallback(defaultState);
}

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

//void NioSocketChannel::write(const BufferPtr& buf)
//{
//	if (state_==kActive)
//	{
//		if (eventLoop_->inEventLoop())
//		{
//			writeAndFlushInLoop(buf, WritePromiseCallbackPtr());
//		}
//		else
//		{
//			eventLoop_->execute(std::move(boost::bind<void,NioSocketChannel,const BufferPtr&,const WritePromiseCallbackPtr>(&NioSocketChannel::writeAndFlushInLoop,this,buf,WritePromiseCallbackPtr())));
//		}
//	}
//}

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
			eventLoop_->execute(std::move(boost::bind<void, NioSocketChannel, const boost::shared_ptr<std::string>&, const WritePromiseCallbackPtr&>(&NioSocketChannel::writeAndFlushInLoop, this, data, WritePromiseCallbackPtr())));
		}
	}
}

void NioSocketChannel::write(const void* msg, size_t len, WritePromiseCallback&& cb)
{
	if (state_==kActive)
	{
		WritePromiseCallbackPtr promise(new WritePromiseCallback(std::move(cb)));
		Buffer* safe = new Buffer();
		safe->append(msg, len);
		if (eventLoop_->inEventLoop())
		{
			writeAndFlushInLoop(safe, promise);
		}
		else
		{
			eventLoop_->execute(std::move(boost::bind<void, NioSocketChannel, Buffer*, const WritePromiseCallbackPtr&>(&NioSocketChannel::writeAndFlushInLoop, this, data, WritePromiseCallbackPtr())));
		}
	}
	else
	{
		eventLoop_->execute(std::move(boost::bind(std::move(cd),shared_from_this(),false)));
	}
}

//void NioSocketChannel::write(const BufferPtr& buf, WritePromiseCallback&& cb)
//{
//	if (state_==kActive)
//	{
//		WritePromiseCallbackPtr promise(new WritePromiseCallback(std::move(cb)));
//		if (eventLoop_->inEventLoop())
//		{
//			writeAndFlushInLoop(buf, promise);
//		}
//		else
//		{
//			eventLoop_->execute(std::move(boost::bind<void, NioSocketChannel, const BufferPtr&, const WritePromiseCallbackPtr&>(&NioSocketChannel::writeAndFlushInLoop, this, buf, promise)));
//		}
//	}
//	else
//	{
//		eventLoop_->execute(std::move(boost::bind(std::move(cb), shared_from_this(), false)));
//	}
//}
	
void NioSocketChannel::write(const boost::shared_ptr<std::string>& data, WritePromiseCallback&& cb)
{
	if (state_ == kActive)
	{
		WritePromiseCallbackPtr promise(new WritePromiseCallback(std::move(cb)));
		if (eventLoop_->inEventLoop())
		{
			writeAndFlushInLoop(data promise);
		}
		else
		{
			eventLoop_->execute(std::move(boost::bind<void, NioSocketChannel, const boost::shared_ptr<std::string>&, const WritePromiseCallbackPtr&>(&NioSocketChannel::writeAndFlushInLoop, this, data, promise)));
		}
	}
	else
	{
		eventLoop_->execute(std::move(boost::bind(std::move(cb), shared_from_this(), false)));
	}
}

//void NioSocketChannel::writeAndFlush(Buffer&& buf)
//{
//	if (state_ == kActive)
//	{
//		Buffer* safe = new Buffer(std::move(buf));
//		if (eventLoop_->inEventLoop())
//		{
//			writeAndFlushInLoop(safe, WritePromiseCallbackPtr());
//		}
//		else
//		{
//			eventLoop_->execute(std::move(boost::bind<void, NioSocketChannel, Buffer*, const WritePromiseCallbackPtr>(&NioSocketChannel::writeAndFlushInLoop, this, safe, WritePromiseCallbackPtr())));
//		}
//	}
//}

void NioSocketChannel::writeAndFlush(std::string* data)
{
	if (state_ == kActive)
	{
		std::string* safe = new std::string();
		if (eventLoop_->inEventLoop())
		{
			writeAndFlushInLoop(safe, WritePromiseCallbackPtr());
		}
		else
		{
			eventLoop_->execute(std::move(boost::bind<void, NioSocketChannel, std::string*, const WritePromiseCallbackPtr>(&NioSocketChannel::writeAndFlushInLoop, this, safe, WritePromiseCallbackPtr())));
		}
	}
}

//void NioSocketChannel::writeAndFlush(Buffer&& buf, WritePromiseCallback&& cb)
//{
//	if (state_ == kActive)
//	{
//		WritePromiseCallbackPtr promise(new WritePromiseCallback(cb));
//		Buffer* safe = new Buffer(std::move(buf));
//		if (eventLoop_->inEventLoop())
//		{
//			writeAndFlushInLoop(safe, promise);
//		}
//		else
//		{
//			eventLoop_->execute(std::move(boost::bind<void, NioSocketChannel, Buffer*, const WritePromiseCallbackPtr>(&NioSocketChannel::writeAndFlushInLoop, this, safe, WritePromiseCallbackPtr())));
//		}
//	}
//	else
//	{
//		eventLoop_->execute(std::move(boost::bind(std::move(cb),shared_from_this(),false)));
//	}
//}

void NioSocketChannel::writeAndFlush(std::string&& data, WritePromiseCallback&& cb)
{
	if (state_ == kActive)
	{
		WritePromiseCallbackPtr promise(new WritePromiseCallback(cb));
		std::string* safe = new std::string(std::move(data));
		if (eventLoop_->inEventLoop())
		{
			writeAndFlushInLoop(safe, promise);
		}
		else
		{
			eventLoop_->execute(std::move(boost::bind<void, NioSocketChannel, std::string*, const WritePromiseCallbackPtr>(&NioSocketChannel::writeAndFlushInLoop, this, safe, WritePromiseCallbackPtr())));
		}
	}
	else
	{
		eventLoop_->execute(std::move(boost::bind(std::move(cb), shared_from_this(), false)));
	}
}

void NioSocketChannel::writeAndFlush(std::string* data, WritePromiseCallback&& cb)
{
	if (state_ == kActive)
	{
		WritePromiseCallbackPtr promise(new WritePromiseCallback(cb));
		std::string* safe = new std::string();
		safe->swap(*data);
		if (eventLoop_->inEventLoop())
		{
			writeAndFlushInLoop(safe, promise);
		}
		else
		{
			eventLoop_->execute(std::move(boost::bind<void, NioSocketChannel, std::string*, const WritePromiseCallbackPtr>(&NioSocketChannel::writeAndFlushInLoop, this, safe, WritePromiseCallbackPtr())));
		}
	}
	else
	{
		eventLoop_->execute(std::move(boost::bind(std::move(cb), shared_from_this(), false)));
	}
}

void NioSocketChannel::writeAndFlush(std::string&& data)
{
	if (state_==kActive)
	{
		std::string* safe = std::string(std::move(data));
		if (eventLoop_->inEventLoop())
		{
			writeAndFlushInLoop(safe, WritePromiseCallbackPtr());
		}
		else
		{
			eventLoop_->execute(std::move(boost::bind<void, NioSocketChannel, std::string*, const WritePromiseCallbackPtr&>(&NioSocketChannel::writeAndFlushInLoop, this, safe, WritePromiseCallbackPtr())));
		}
	}
}
	
//void NioSocketChannel::writeAndFlush(Buffer* buf)
//{
//	if (state_ == kActive)
//	{
//		Buffer* safe = new Buffer();
//		safe->safe(*buf);
//		std::string* safe = std::string(std::move(data));
//		if (eventLoop_->inEventLoop())
//		{
//			writeAndFlushInLoop(safe, WritePromiseCallbackPtr());
//		}
//		else
//		{
//			eventLoop_->execute(std::move(boost::bind<void, NioSocketChannel, Buffer*, const WritePromiseCallbackPtr&>(&NioSocketChannel::writeAndFlushInLoop, this, safe, WritePromiseCallbackPtr())));
//		}
//	}
//}
	
//void NioSocketChannel::writeAndFlush(Buffer* buf, WritePromiseCallback&& cb)
//{
//	if (state_ == kActive)
//	{
//		WritePromiseCallbackPtr promise(new WritePromiseCallback(cb));
//		Buffer* safe = new Buffer();
//		safe->safe(*buf);
//		if (eventLoop_->inEventLoop())
//		{
//			writeAndFlushInLoop(safe, promise);
//		}
//		else
//		{
//			eventLoop_->execute(std::move(boost::bind<void, NioSocketChannel, Buffer*, const WritePromiseCallbackPtr&>(&NioSocketChannel::writeAndFlushInLoop, this, safe, WritePromiseCallbackPtr())));
//		}
//	}
//}
	
void NioSocketChannel::shutdown()
{
	if (state_==kActive)
	{
		setChannelState(kInactiving);
		eventLoop_->execute(std::move(boost::bind(&NioSocketChannel::shutdownInLoop,this)));
	}
}

void NioSocketChannel::close()
{
	if (state_==kActive || state_ ==kInactiving)
	{
		setChannelState(kInactiving);
		eventLoop_->execute(std::move(boost::bind(&NioSocketChannel::closeInLoop, this)));
	}
}

void NioSocketChannel::closeWtihDelay(double seconds)
{
	if (state_ == kActive || state_ == kInactiving)
	{
		setChannelState(kInactiving);
		eventLoop_->schedualOnce(makeWeakCallback(shared_from_this(),&NioSocketChannel::close), seconds);
	}
}

void NioSocketChannel::established()
{
	assert(state_ == kActiving);
	setChannelState(kActive);
	NioSocketChannelPtr guard(shared_from_this());
	pipeline_->tie(guard);
	pipeline_->enableRead();
	pipeline_->channelActive(guard);
}

void NioSocketChannel::destroyed()
{
	if (state_==kActive)
	{
		setChannelState(kInactive);
		pipeline_->disableAll();
		pipeline_->channelInActive(shared_from_this());
	}
	pipeline_.reset();
}

//void NioSocketChannel::writeAndFlushInLoop(Buffer* buf, const WritePromiseCallbackPtr& promise)
//{
//	if (state_==kInactive)
//	{
//		std::_Count_pr << "channel " << name_ << " inactive when write in it's internal eventloop" << std::endl;
//		(*promise)(shared_from_this(), false);
//		return;
//	}
//	if (promise)
//	{
//		pipeline_->pushPromise(promise);
//	}
//	Bytebuf* output = pipeline_->output();
//	output->appendReference(buf, true);
//}
	
void NioSocketChannel::writeAndFlushInLoop(std::string* data, const WritePromiseCallbackPtr& promise)
{
	if (state_ == kInactive)
	{
		std::_Count_pr << "channel " << name_ << " inactive when write in it's internal eventloop" << std::endl;
		(*promise)(shared_from_this(), false);
		return;
	}
	if (promise)
	{
		pipeline_->pushPromise(promise);
	}
	Bytebuf* output = pipeline_->output();
	output->appendReference(data, true);
}

//void NioSocketChannel::writeAndFlushInLoop(const BufferPtr& buf, const WritePromiseCallbackPtr& promise)
//{
//	if (state_ == kInactive)
//	{
//		std::_Count_pr << "channel " << name_ << " inactive when write in it's internal eventloop" << std::endl;
//		(*promise)(shared_from_this(), false);
//		return;
//	}
//	if (promise)
//	{
//		pipeline_->pushPromise(promise);
//	}
//	Bytebuf* output = pipeline_->output();
//	output->appendReference(buf);
//}

void NioSocketChannel::writeAndFlushInLoop(const boost::shared_ptr<std::string>& data, const WritePromiseCallbackPtr& promise)
{
	if (state_ == kInactive)
	{
		std::_Count_pr << "channel " << name_ << " inactive when write in it's internal eventloop" << std::endl;
		(*promise)(shared_from_this(), false);
		return;
	}
	if (promise)
	{
		pipeline_->pushPromise(promise);
	}
	Bytebuf* output = pipeline_->output();
	output->appendReference(data);
}

void NioSocketChannel::shutdownInLoop()
{
	eventLoop_->assertInLoopThread();
	if (!pipeline_->isWritting())
	{
		socket_.shutdownWrite();
	}
}
	
void NioSocketChannel::closeInLoop()
{
	if (state_==kActive || state_==kInactiving)
	{
		handleClose();
	}
}

void NioSocketChannel::handleClose()
{
	setChannelState(kInactive);
	pipeline_->disableAll();
	NioSocketChannelPtr guard(shared_from_this());
	pipeline_->channelInActive(guard);
	pipeline_->close(guard);
}
	
void NioSocketChannel::handleError(bool invokeWritePromise)
{
	int errcode = evutil_socket_geterror(socket_.fd());
	const char* errmsg = evutil_socket_error_to_string(errcode);
	std::cout << "channel " << name_ << "caught error(errno=" << errcode << " " << errmsg << ")" << std::endl;
	switch (errno)
	{
	case EPIPE:
	case ECONNRESET:
		pipeline_->exceptionCaught(shared_from_this(), invokeWritePromise);
		break;
	case EINTR:
	case EAGAIN:

		break;
	default:
		break;
	}
}

const char* NioSocketChannel::stateToString() const
{
	switch (state_)
	{
	case NioSocketChannel::kInactive:
		return "Inactive";
	case NioSocketChannel::kActiving:
		return "Activing";
	case NioSocketChannel::kActive:
		return "Active";
	case NioSocketChannel::kInactiving:
		return "Inactiving";
	default:
		break;
	}
	return "Unknow state";
}

NioSocketChannelPtr NioSocketChannel::Builder::build()
{
	return NioSocketChannelPtr(new NioSocketChannel(eventLoop_,id_,std::move(name_),sockfd_,remote_,local_,inspector_));
}