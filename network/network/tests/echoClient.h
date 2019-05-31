#ifndef __ECHOCLIENT_H__
#define __ECHOCLIENT_H__
#include "../NioEventLoop.h"
#include"../ChannelInitailizer.h"
#include "../ClientBootstrap.h"
#include "../InetSocketAddress.h"
#include <iostream>
#include <string>

class EchoClient
{
public:
	EchoClient(NioEventLoop* evnetLoop, bool retry)
		:eventLoop_(evnetLoop)
		,bootstrap_(evnetLoop,"EchoClient",InetSocketAddress("192.168.41.136",10003))
		,task_()
		,sequence_(0)
	{
		bootstrap_.enableRetry(retry)
			.channelInitCallback([this](const ChannelInitailizerPtr& channelInitailizer)
		{
			channelInitailizer->channelActiveCallback(boost::bind(&EchoClient::channeActive,this,_1));
			channelInitailizer->channelInactiveCallback(boost::bind(&EchoClient::channeInActive, this, _1));
			channelInitailizer->messageCallback(boost::bind(&EchoClient::messageArrived, this, _1,_2,_3));
		})
			.option(OPT_TCPNODELAY,true)
			.option(OPT_KEEPALIVE,true)
			.option(OPT_WRITEIDELETIMEOUT,300);
	}

	~EchoClient(){}

	void start()
	{
		bootstrap_.connect();
	}

	void channeActive(const NioSocketChannelPtr& channel)
	{
		std::cout << "channeActive:" << channel->channelName() << std::endl;
		std::string msg(1024 * 1024 * 10, 'a');
		size_t msgLen = msg.size();
		BufferPtr buf(new Buffer());
		buf->appendInt64(msgLen);
		buf->appendString(msg);

		eventLoop_->schedualEvery(boost::bind<void,NioSocketChannel,const BufferPtr&>(&NioSocketChannel::write,channel,buf),7.0);
	}


	void channeInActive(const NioSocketChannelPtr& channel)
	{
		std::cout << "channeInActive " << channel->channelName() << std::endl;
		sequence_ = 0;
	}

	void messageArrived(const NioSocketChannelPtr& channel, Bytebuf& input, Timestamp recvTime)
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
			std::cout << "client:data recv " << dataLen << std::endl;

			std::string data;
			data.resize(dataLen);
			input.readBytes(const_cast<char*>(data.data()), dataLen);

			if (++sequence_>=10)
			{
				channel->close();
			}
		}
	}

private:
	NioEventLoop* eventLoop_;
	ClientBootstrap bootstrap_;
	TimerId task_;
	size_t sequence_;
};

void echoClientMain()
{
	std::cout << "pid=" << getpid() << " tid=" << CurrentThread::tid() << std::endl;
	NioEventLoop mainLoop;
	EchoClient c(&mainLoop, true);
	c.start();

	mainLoop.loop();
}

#endif
