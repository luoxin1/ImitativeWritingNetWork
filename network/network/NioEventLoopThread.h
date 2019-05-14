#ifndef __NIOEVENTLOOPTHREAD_H__
#define __NIOEVENTLOOPTHREAD_H__
#include"boost/function.hpp"
#include"NioEventLoop.h"

class NioEventLoopThread
{
public:
	typedef boost::function<void(NioEventLoop* eventLoop)> ThreadInitCallback;

	NioEventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback());
	~NioEventLoopThread();

	NioEventLoop* startThread();
	void stopThread();

private:
	void threadFunc();

	NioEventLoop* eventLoop_;
	bool exiting_;
	boost::scope_ptr<boost::thread> thread_;
	mutable boost::mutex mutex_;
	boost::condition cond_;
	ThreadInitCallback callback_;
};

typedef boost::shared_ptr<NioEventLoopGroup> NioEventLoopGroupPtr;

#endif // !__NIOEVENTLOOPTHREAD_H__

