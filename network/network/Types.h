#ifndef __TYPES_H__
#define __TYPES_H__
#include "boost/function.hpp"

class ChannelInitailizer;
typedef boost::shared_ptr<ChannelInitailizer> ChannelInitailizerPtr;
typedef boost::function<void(const ChannelInitailizerPtr& channelInitailizer)> ChannelInitCallback;


class NioSocketChannel;
typedef boost::shared_ptr<NioSocketChannel> NioSocketChannelPtr;

class IdelChannelInspector;
typedef boost::shared_ptr<IdelChannelInspector> IdelChannelInspectorPtr;


class Listener;
typedef boost::shared_ptr<Listener> ListenerPtr;

#endif
