#ifndef __NIOEVENTLOOPGROUP_H__
#define __NioEventLoopGroup_h__
#include <vector>
#include"boost/function.hpp"
#include"NioEventLoop.h"

class NioEventLoopThread;

class NioEventLoopGroup
{
public:
	typedef NioEventLoop::LoadOption LoadOption;
	typedef boost::function<void(NioEventLoop* eventLoop)> ThreadInitCallback;

	NioEventLoopGroup(NioEventLoop* baseLoop,const std::string& nameArg,const ThreadInitCallback& cb= ThreadInitCallback());

	~NioEventLoopGroup();

	void setNumThreads(size_t numThreads)
	{
		numThreads_ = numThreads;
	}

	void start();

	NioEventLoop* next();
	NioEventLoop* lightWeighted(LoadOption opt);
	NioEventLoop* specialForHash(size_t hashCode);
	std::vector<NioEventLoop*> allLoop();

	bool started() const { return started_; }
	const std::string& name() const { return name_; }
	NioEventLoop* baseLoop() const { return baseLoop_; }
private:
	struct Balance
	{
		bool operator()(NioEventLoop* lhs, NioEventLoop* rhs) const;
	};

	typedef boost::shared_ptr<NioEventLoopThread> NioEventLoopThreadPtr;
	typedef std::vector<NioEventLoop*> NioEventLoopList;
	typedef std::priority_queue<NioEventLoop* NioEventLoopList, Balance> NioEventLoopPriorityQueue;

	NioEventLoop* baseLoop_;

	const std::string name_;
	const ThreadCallback threadInit_;

	size_t numThreads_;

	boost::atomic_bool started_;
	boost::atomic_uint64_t next_;
	std::vector<NioEventLoopThreadPtr> threads_;
	NioEventLoopList loops_;

	boost::mutex  mutex_;
};

typedef boost::shared_ptr<NioEventLoopGroup> NioEventLoopGroupPtr;

#endif
