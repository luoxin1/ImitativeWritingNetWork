#include "NioEventLoopGroup.h"
#include "NioEventLoopThread.h"
#include<iostream>

NioEventLoopGroup::NioEventLoopGroup(NioEventLoop* baseLoop, const std::string& nameArg, const ThreadInitCallback& cb)
	:baseLoop_(baseLoop)
	,name_(nameArg)
	,threadInit_(cb)
	, numThreads_(0)
	,started_(false)
	,next_(0)
	,threads_()
	,loops_()
	,mutex_()
{
	assert(baseLoop_ != NULL);
}

NioEventLoopGroup::~NioEventLoopGroup()
{

}

void NioEventLoopGroup::start()
{
	assert(!started_);
	baseLoop_->assertInLoopThread();
	started_ = true;

	for (size_t i=0;i<numThreads_;++i)
	{
		NioEventLoopThreadPtr t(new NioEventLoopThread(threadInit_));
		threads_.push_back(t);

		NioEventLoop* loop = t->startThread();
		loop->owerGroup(this);
		loops_.push_back(loop);
	}
	if (numThreads_==0 && threadInit_)
	{
		threadInit_(baseLoop_);
	}
}

NioEventLoop* NioEventLoopGroup::next()
{
	assert(started_);
	NioEventLoop* loop = baseLoop_;
	if (!loops_.empty())
	{
		size_t i =next_++;
		loop = loops_[i%loops_.size()];
	}
	return loop;
}

NioEventLoop* NioEventLoopGroup::lightWeighted(LoadOption opt)
{
        std::cout<<"5555555555555"<<std::endl;
	assert(started_);
	NioEventLoop* loop = baseLoop_;
        boost::lock_guard<boost::mutex> lock(mutex_);
	if (!loops_.empty())
	{
		NioEventLoopPriorityQueue weightedLoops(loops_.begin(), loops_.end());
		loop = weightedLoops.top();
	}
	loop->rebalance(opt);
	return loop;
}

NioEventLoop* NioEventLoopGroup::specialForHash(size_t hashCode)
{
	assert(started_);
	NioEventLoop* loop = baseLoop_;
	if (!loops_.empty())
	{
		loop = loops_[hashCode % loops_.size()];
	}
	return loop;
}

std::vector<NioEventLoop*> NioEventLoopGroup::allLoop()
{
	assert(started_);
	if (loops_.empty())
	{
		return std::vector<NioEventLoop*>(1, baseLoop_);
	}
	return loops_;
}

bool NioEventLoopGroup::Balance::operator()(NioEventLoop* lhs, NioEventLoop* rhs) const
{
	return lhs->loadfactor() > rhs->loadfactor();
}