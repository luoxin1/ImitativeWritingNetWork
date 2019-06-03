#include "ChannelPipeline.h"
#include "event2/bufferevent.h"
#include"IdlChanelInspector.h"
#include<iostream>
#include"event2/bufferevent.h"
#include"NioSocketChannel.h"

ChannelPipeline::ChannelPipeline(NioEventLoop* eventLoop, evutil_socket_t sockfd, int flag)
	:eventLoop_(eventLoop)
	,underlying_(bufferevent_socket_new(NioEventLoop::Unsafe(eventLoop).entrieBase(),sockfd,flag))
	,input_(bufferevent_get_input(underlying_),false)
	,output_(bufferevent_get_output(underlying_),false)
	,readWaterMark_(std::make_pair(0,0))
	,idleStateTimeouts_(std::make_pair(0,0))
	,writePromis_()
	,self_()
	,selfChannel_()
	,interestIdles_()
	,channelActive_()
	,channelInActive_()
	,messageReceived_()
	,idleState_()
	,channelClose_()
{
	assert(underlying_ != NULL);
}

ChannelPipeline::~ChannelPipeline()
{
	bufferevent_free(underlying_);
}

ChannelPipeline& ChannelPipeline::option(ChannelOption opt, int optval)
{
	switch (opt)
	{
	case OPT_READLOWWATERMARK:
		readWaterMark_.first = optval;
		break;
	case OPT_READHIGHWATERMARK:
		readWaterMark_.second = optval;
		break;
	case OPT_READIDLEKETIMEOUT:
		interestIdles_.insert(std::make_pair(READ_IDLE, WeakChannelEntryPtr()));
		break;
	case OPT_WRITEIDELETIMEOUT:
		interestIdles_.insert(std::make_pair(WRITE_IDLE, WeakChannelEntryPtr()));
		break;
	case OPT_ALLIDELRTIMEOUT:
		interestIdles_.insert(std::make_pair(ALL_IDLE, WeakChannelEntryPtr()));
		break;
	default:
		break;
	}
	return *this;
}

void ChannelPipeline::tie(const NioSocketChannelPtr& selfChannel)
{
        std::cout<<"10@@@@@@@@ ChannelPipeline::tie bufferevent_setcb"<<std::endl;
	self_ = shared_from_this();
	selfChannel_ = selfChannel;
	std::for_each(interestIdles_.begin(), interestIdles_.end(), [&selfChannel](std::pair<const IdleState, WeakChannelEntryPtr>& it)
	{
		ChannelEntryPtr entry(new ChannelEntry(selfChannel, it.first));
		it.second = entry;
		selfChannel->inspector_->backInsert(entry, it.first);
	});
	bufferevent_setcb(underlying_, messageReceived, writePromise, eventCaught, reinterpret_cast<void*>(&self_));
}

void ChannelPipeline::enableRead()
{
        std::cout<<"8@@@@@@@@ ChannelPipeline::enableRead"<<std::endl;
	if (readWaterMark_.first < readWaterMark_.second && readWaterMark_.second >= 0)
	{
		bufferevent_setwatermark(underlying_, EV_READ, readWaterMark_.first, readWaterMark_.second);
	}
	bufferevent_enable(underlying_, EV_READ);
}

void ChannelPipeline::enableWrite()
{
	bufferevent_enable(underlying_, EV_WRITE);
}

void ChannelPipeline::enableAll()
{
	enableRead();
	enableWrite();
}

void ChannelPipeline::disableRead()
{
	bufferevent_disable(underlying_, EV_READ);
}

void ChannelPipeline::disableWrite()
{
	bufferevent_disable(underlying_, EV_WRITE);
}

void ChannelPipeline::disableAll()
{
	bufferevent_disable(underlying_, EV_READ | EV_WRITE);
}

bool ChannelPipeline::isReading()
{
	return bufferevent_get_enabled(underlying_)& EV_READ;
}

bool ChannelPipeline::isWritting()
{
	return bufferevent_get_enabled(underlying_)& EV_WRITE;
}

bool ChannelPipeline::isNonEvent()
{
	return bufferevent_get_enabled(underlying_) == 0;
}

void ChannelPipeline::channelActive(const NioSocketChannelPtr& channel)
{
        std::cout<<"11@@@@@@@@ ChannelPipeline::channelActive"<<std::endl;
	channelActive_(channel);
}

void ChannelPipeline::channelInActive(const NioSocketChannelPtr& channel)
{
	channelInActive_(channel);
}

void ChannelPipeline::idleStateTriggered(const NioSocketChannelPtr& channel, IdleState idleState)
{
	idleState_(channel, idleState);
}

void ChannelPipeline::close(const NioSocketChannelPtr& channel)
{
	channelClose_(channel);
}

void ChannelPipeline::exceptionCaught(const NioSocketChannelPtr& channel, bool invokeWritePromiss)
{
	if (invokeWritePromiss && !writePromis_.empty())
	{
		std::cout<< "error occured while writting!" << std::endl;
		std::for_each(writePromis_.begin(), writePromis_.end(), [&channel](const WritePromiseCallbackPtr& promise)
		{
			(*promise)(channel, false);
		});
                writePromis_.clear();
	}
	channel->close();
}

void ChannelPipeline::messageReceived(struct bufferevent* be, void* privdata)
{
	boost::weak_ptr<ChannelPipeline>* pipeline = static_cast<boost::weak_ptr<ChannelPipeline>*>(privdata);
	ChannelPipelinePtr self(pipeline->lock());
	if (self)
	{
                std::cout<<"12 ChannelPipeline::messageReceived"<<std::endl;
		NioSocketChannelPtr selfChannel(self->selfChannel_.lock());
		self->messageReceived_(selfChannel, self->input_, self->eventLoop_->pollReturnTime());

		self->updateIdleChannelEntry(selfChannel, ALL_IDLE);
		self->updateIdleChannelEntry(selfChannel, READ_IDLE);
	}
}

void ChannelPipeline::writePromise(struct bufferevent* be, void* privdata)
{
	boost::weak_ptr<ChannelPipeline>* pipeline = static_cast<boost::weak_ptr<ChannelPipeline>*>(privdata);
	ChannelPipelinePtr self(pipeline->lock());
	if (self)
	{
		NioSocketChannelPtr selfChannel(self->selfChannel_.lock());
		if (!self->writePromis_.empty())
		{
			std::deque<WritePromiseCallbackPtr>& promises = self->writePromis_;
			std::for_each(promises.begin(), promises.end(), [&selfChannel](const WritePromiseCallbackPtr& promise)
			{
				(*promise)(selfChannel, true);
			});
			promises.clear();
		}
		self->updateIdleChannelEntry(selfChannel, ALL_IDLE);
		self->updateIdleChannelEntry(selfChannel, WRITE_IDLE);
	}
}

void ChannelPipeline::eventCaught(struct bufferevent* be, short what, void* privdata)
{
	boost::weak_ptr<ChannelPipeline>* pipeline = static_cast<boost::weak_ptr<ChannelPipeline>*>(privdata);
	ChannelPipelinePtr self(pipeline->lock());
	if (self)
	{
		NioSocketChannelPtr selfChannel(self->selfChannel_.lock());
		do
		{
			if (what & BEV_EVENT_EOF)
			{
				selfChannel->handleClose();
				break;
			}
			if (what & BEV_EVENT_ERROR)
			{
				selfChannel->handleError();
				break;
			}
			if (what & BEV_EVENT_CONNECTED)
			{
				selfChannel->established();
				break;
			}
			if (what & BEV_EVENT_READING)
			{
				selfChannel->handleError();
				break;
			}
			if (what & BEV_EVENT_WRITING)
			{
				selfChannel->handleError();
				break;
			}
		} while (false);
	}

}

void ChannelPipeline::updateIdleChannelEntry(const NioSocketChannelPtr& selfChannel, IdleState ildestate)
{
	auto it = interestIdles_.find(ildestate);
	if (it!=interestIdles_.end())
	{
		ChannelEntryPtr entry(it->second.lock());
		if (entry)
		{
			selfChannel->inspector_->backInsert(entry, ildestate);
		}
	}
}
