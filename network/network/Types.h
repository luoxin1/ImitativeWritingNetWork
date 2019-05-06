#ifndef __TYPES_H__
#define __TYPES_H__
#include "boost/function.hpp"
#include"ByteBuf.h"
#include"boost/shared_ptr.hpp"

enum IdleState
{
	READ_IDLE=0,
	WRITE_IDLE,
	ALL_IDLE
};

class ChannelInitailizer;
typedef boost::shared_ptr<ChannelInitailizer> ChannelInitailizerPtr;
typedef boost::function<void(const ChannelInitailizerPtr& channelInitailizer)> ChannelInitCallback;


class NioSocketChannel;
typedef boost::shared_ptr<NioSocketChannel> NioSocketChannelPtr;

class IdelChannelInspector;
typedef boost::shared_ptr<IdelChannelInspector> IdelChannelInspectorPtr;


class Listener;
typedef boost::shared_ptr<Listener> ListenerPtr;

typedef boost::function<void(const ChannelInitailizerPtr& channelInitailizerPtr)> ChannelActiveCallback;
typedef boost::function<void(const ChannelInitailizerPtr& channelInitailizerPtr)> ChannelInActiveCallback;

typedef boost::function<void(const NioSocketChannelPtr& channel, Bytebuf& input, Timestamp timestamp)> MessageCallback;
typedef boost::function<void(const NioSocketChannelPtr& channel, IdleState idleState)> IdleStateCallback;
typedef boost::function<void(const NioSocketChannelPtr& channel)> ChannelCloseCallback;

typedef boost::function<void(const NioSocketChannelPtr& channel, bool success)> WritePromiseCallback;
typedef boost::shared_ptr<WritePromiseCallback> WritePromiseCallbackPtr;

typedef boost::bad_weak_ptr<NioSocketChannel> WeakNioSocketChannelPtr;

#endif
