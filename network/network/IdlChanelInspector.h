#ifndef __IDLCHANELINSPECTOR_H__
#define __IDLCHANELINSPECTOR_H__
#include <vector>
#include "Types.h"

struct ChannelEntry
{
	explicit ChannelEntry(const NioSocketChannelPtr& channel, IdleState idleState);
	~ChannelEntry();

	WeakNioSocketChannelPtr channel_;
	IdleState idleState_;
	size_t position_;
};

typedef boost::shared_ptr<ChannelEntry> ChannelEntryPtr;
typedef boost::weak_ptr<ChannelEntry> WeakChannelEntryPtr;
typedef boost::unordered_set<ChannelEntryPtr> Bucket;
typedef boost::circular_buffer<Bucket> WeakChannelList;
typedef boost::shared_ptr<WeakChannelList> WeakChannelListPtr;
typedef boost::circular_buffer<Bucket>::iterator WeakChannelListIterator;

class IdlChanelInspector :boost::noncopyable
{
public:
	IdlChanelInspector(NioEventLoop* eventLoop,
		size_t readerIdlSeconds,
		size_t writeIdlSeconds,
		size_t allIdleSeconds);

	~IdlChanelInspector();

	void backInsert(const ChannelEntry& channelEntry, IdleState idlState)
	{
		BucketTableElement& elem = idleBucketTable_[idlState];
		if (elem.channelList_ && channelEntry->position_!=elem.position_)
		{
			elem.channelList_->back().insert(channelEntry);
			channelEntry->position_ = elem.position_;
		}
	}

private:
	void inspect();

	struct BucketTableElement
	{
		WeakChannelListPtr channelList_;
		size_t position_;

		BucketTableElement()
			:channelList_()
			, position_()
		{

		}
	};

	TimerId inspectId_;
	std::vector<BucketTableElement> idleBucketTable_;
};

typedef boost::shared_ptr<IdlChanelInspector> IdlChanelInspectorPtr;

#endif
