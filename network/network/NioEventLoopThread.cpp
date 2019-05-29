#include "NioEventLoopThread.h"
#include"NioEventLoop.h"

NioEventLoopThread::NioEventLoopThread(const ThreadInitCallback& cb)
	:eventLoop_(NULL)
	,exiting_(false)
	,thread_()
	,mutex_()
	,cond_(),
	callback_(cb)
{

}

NioEventLoopThread::~NioEventLoopThread()
{
	stopThread();
}

NioEventLoop* NioEventLoopThread::startThread()
{
	thread_.reset(new boost::thread(boost::bind(&NioEventLoopThread::threadFunc, this)));
	{
		boost::unique_lock<boost::mutex> lock(mutex_);
		while (eventLoop_==NULL)
		{
			cond_.wait(lock);
		}
	}
	return eventLoop_;
}


void NioEventLoopThread::stopThread()
{
	exiting_ = true;
	if (eventLoop_!=NULL)
	{
		eventLoop_->shutdownGracefully();
		thread_->join();
	}
}

void NioEventLoopThread::threadFunc()
{
	NioEventLoop eventLoop;
	if (callback_)
	{
		callback_(&eventLoop);
	}
	{
		boost::unique_lock<boost::mutex> lock(mutex_);
		eventLoop_ = &eventLoop;
		cond_.notify_one();
	}
	eventLoop.loop();
	eventLoop_ = NULL;
}