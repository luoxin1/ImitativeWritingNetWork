#ifndef __ECHOSERVER_H__
#define __ECHOSERVER_H__
#include "../ServerBootstrap.h"
#include "../NioEventLoop.h"
#include"../ChannelInitailizer.h"
#include "../Timestamp.h"
#include "../CurrentThread.h"
#include<iostream>
#include <string>

class EchoServer
{
public:
	EchoServer(NioEventLoop* baseLoop, uint16_t port)
		:baseLoop_(baseLoop)
		,bootstrap_(baseLoop,"echoServer",InetSocketAddress(port))
		,group_(new NioEventLoopGroup(baseLoop_,"EchoServerWorkGroup"))
	{
		bootstrap_.group(group_)
			.chanelInitCallback([this](const ChannelInitailizerPtr& channelInitailizer)
		{
			//typedef boost::function<void(const NioSocketChannelPtr& channelInitailizerPtr)> ChannelActiveCallback;
			channelInitailizer->channelActiveCallback(boost::bind(&EchoServer::channelActive,this,_1));
			channelInitailizer->channelInactiveCallback(boost::bind(&EchoServer::channelInActive, this, _1));
			channelInitailizer->messageCallback(boost::bind(&EchoServer::messageArrived, this, _1, _2, _3));
		})
			.option(OPT_KEEPALIVE,true)
			.option(OPT_TCPNODELAY,true);
	}

	~EchoServer() {}

	void start()
	{
		bootstrap_.start();
	}

	void channelActive(const NioSocketChannelPtr& channel)
	{
		std::cout << "channelActive" << channel->channelName() << std::endl;
	}

	void channelInActive(const NioSocketChannelPtr& channel)
	{
		std::cout << "channelInActive" << channel->channelName() << std::endl;
	}

	void messageArrived(const NioSocketChannelPtr& channel, Bytebuf& input, Timestamp reciveTime)
	{
		while (input.readalbeBytes()>=8)
		{
			size_t dataLen = 0;
			input.peekBytes(&dataLen, sizeof(dataLen));
			if (input.readalbeBytes()<dataLen+8)
			{
				break;
			}

			input.discardBytest(8);

			std::string data;
			data.resize(dataLen);

			int ret = input.readBytes(const_cast<char*>(data.data()),dataLen);
			std::cout << "server:recv [ " << data.size() << "] bytes arrived at " << reciveTime.toFormattedString() << std::endl;

			Buffer buf;
			buf.appendInt64(dataLen);
			buf.appendString(data);

			channel->writeAndFlush(&buf,[](const NioSocketChannelPtr& ch, bool success)
			{
				if (success)
				{
					std::cout << "result " << (success ? "TRUE" : "FALSE") << std::endl;;
				}
			});
			
		}
	}
private:
	NioEventLoop* baseLoop_;
	ServerBootstrap bootstrap_;
	NioEventLoopGroupPtr group_;
};

void echoServerMain()
{
	std::cout << "start server " << CurrentThread::tid()<<" listen port is 10000" << std::endl;
	NioEventLoop mainLoop;

	EchoServer s(&mainLoop, 10000);
	s.start();

	mainLoop.loop();

}

#endif
