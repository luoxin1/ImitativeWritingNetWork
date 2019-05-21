#ifndef __NIOEVENTLOOPTHREAD_H__
#define __NIOEVENTLOOPTHREAD_H__
#include"boost/function.hpp"
#include"boost/smart_ptr.hpp"
#include"boost/thread.hpp"
#include"boost/thread/condition.hpp"

class NioEventLoop;

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
	boost::scoped_ptr<boost::thread> thread_;
	mutable boost::mutex mutex_;
	boost::condition cond_;
	ThreadInitCallback callback_;
};

#endif // !__NIOEVENTLOOPTHREAD_H__

