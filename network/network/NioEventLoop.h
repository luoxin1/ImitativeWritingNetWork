#ifndef __NIOEVENTLOOP_H__
#define __NIOEVENTLOOP_H__
#include "Timestamp.h"
#include "TimerId.h"
#include"boost/atomic.hpp"
#include"CurrentThread.h"
#include "boost/function.hpp"
#include "boost/noncopyable.hpp"

struct event_base;
class NioEventLoopGroup;

class NioEventLoop:public boost::noncopyable
{
public:
	enum LoadOption
	{
		kReadOnly = 0,
		kAdd,
		kDelete
	};

	typedef boost::function<void()> NioTask;

	NioEventLoop();
	~NioEventLoop();


	void loop();
	void shutdownGracefully(Timestamp time = Timestamp());

	Timestamp pollReturnTime() const;

	void execute(const NioTask& task);
	void execute(NioTask&& task);

	void scheduleOnce(const NioTask& task, double defferdTime);
	void schedualOnce(NioTask&& task, double defferedTime);

	TimerId schedualAt(const NioTask& task, Timestamp time);
	TimerId schedualAfter(const NioTask& task, double delay);
	TimerId schedualEvery(const NioTask& task, double interval);

	TimerId schedualAt(const NioTask&& task, Timestamp time);
	TimerId schedualAfter(const NioTask&& task, double delay);
	TimerId schedualEvery(const NioTask&& task, double interval);

	void cancle(TimerId id);

	size_t rebalance(LoadOption option);
	NioEventLoopGroup* owerGroup(NioEventLoopGroup* group);

	pid_t threadId() const
	{
		return threadId_;
	}

	size_t loadfactor() const 
	{
		return loadfactor_;
	}

	bool inEventLoop() const
	{
		return threadId_ == CurrentThread::tid();
	}

	void assertInLoopThread()
	{
		if (!inEventLoop())
		{
			assertInLoopThread();
		}
	}

	static NioEventLoop* getEventLoopOfCurrentThread();

	class Unsafe :boost::noncopyable
	{
	public:
		Unsafe(NioEventLoop* selfEventLoop)
			:selfEventLoop_(selfEventLoop)
		{

		}

		~Unsafe(){}

		struct event_base* entrieBase() const 
		{
			selfEventLoop_->base_;
		}

	private:
		NioEventLoop* selfEventLoop_;
	};

private:
	void abortNotInLoopThread();

	const size_t index_;
	const pid_t threadId_;
	struct event_base* base_;
	NioEventLoopGroup* ownerGroup_;
	const bool isMainLoop_;
	size_t timeSeq_;
	boost::atomic_int64_t loadfactor_;

	static boost::atomic_bool setupReactorContext;
	static boost::atomic_uint64_t loopIndex;
};
#endif
