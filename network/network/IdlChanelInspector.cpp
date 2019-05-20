#include"IdlChanelInspector.h"
#include"ChannelPipeline.h"

ChannelEntry::ChannelEntry(const NioSocketChannelPtr& channel, IdleState idleState)
	:channel_(channel)
	,idleState_(idleState)
	,position_(0)
{

}

ChannelEntry::~ChannelEntry()
{
	NioSocketChannelPtr channel = channel_.lock();
	if (channel)
	{
		ChannelPipelinePtr pipleline = channel->pipeline_;
		if (pipleline)
		{
			channel->idleStateTrigger(channel, idleState_);
		}
	}
}

static const size_t kNumIdleTypes = 3;

IdlChanelInspector::IdlChanelInspector(NioEventLoop* eventLoop,
	size_t readerIdlSeconds,
	size_t writeIdlSeconds,
	size_t allIdleSeconds)
	:inspectId_()
	,idleBucketTable_(kNumIdleTypes,BucketTableElement())
{
	if (readerIdlSeconds > 0)
	{
		idleBucketTable_[READ_IDLE].channelList_.reset(new WeakChannelList());
		idleBucketTable_[READ_IDLE].channelList_.resize(readerIdlSeconds);
		idleBucketTable_[READ_IDLE].position_=readerIdlSeconds-1;
	}
	if (writeIdlSeconds>0)
	{
		idleBucketTable_[READ_IDLE].channelList_.reset(new WeakChannelList());
		idleBucketTable_[READ_IDLE].channelList_.resize(writeIdlSeconds);
		idleBucketTable_[READ_IDLE].position_ = writeIdlSeconds - 1;
	}
	if (allIdleSeconds>0)
	{
		idleBucketTable_[READ_IDLE].channelList_.reset(new WeakChannelList());
		idleBucketTable_[READ_IDLE].channelList_.resize(allIdleSeconds);
		idleBucketTable_[READ_IDLE].position_ = allIdleSeconds - 1;
	}
	inspectId_ = eventLoop->schedualEvery(boost::bind(&IdlChanelInspector::inspect, this), 1.0);
}

IdlChanelInspector::~IdlChanelInspector()
{
	inspectId_.cancel();
}


void IdlChanelInspector::inspect()
{
	std::for_each(idleBucketTable_.begin(), idleBucketTable_.end(), [](BucketTableElement& elm)
	{
		if (elm.channelList_)
		{
			elm.channelList_->push_back(Bucket());
			++elm.position_;
		}

	});
}