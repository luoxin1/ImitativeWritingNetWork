#include <signal.h>
#include <execinfo.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include "NioEventLoop.h"
#include "Functor.h"
#include"event2/util.h"
#include"event2/event.h"
#include<iostream>

__thread NioEventLoop* t_loopInThisThread = NULL;

void changeLimit()
{
	struct rlimit r;
	r.rlim_cur = RLIM_INFINITY;
	r.rlim_max = RLIM_INFINITY;
	setrlimit(RLIMIT_NOFILE, &r);
	setrlimit(RLIMIT_CORE, &r);
}

void sigterm_cb(evutil_socket_t signo,short event,void* privdata)
{
	struct event* e = static_cast<struct event*>(privdata);
	evsignal_del(e);
	exit(0);
}

void sigsegv_cb(evutil_socket_t signo, short event, void* privdata)
{
	struct event* e = static_cast<struct event*>(privdata);
	evsignal_del(e);
	void* array[100];
	char** strings;
	signal(signo, SIG_DFL);
	size_t size = backtrace(array, 100);
	strings = backtrace_symbols(array, size);
	if (strings!=NULL)
	{
		char strTime[32] = { 0 };
		sprintf(strTime, "core.bt.%d", getpid());
		FILE* fp = fopen(strTime, "w");
		if (fp!=NULL)
		{
			for (size_t i=0;i<size;++i)
			{
				fputs(strings[i], fp);
				fputs("\n", fp);
			}
			fclose(fp);
		}
		free(strings);
	}
}

void setupSignalHandlers(struct event_base* base)
{
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGABRT, SIG_IGN);
	signal(SIGBUS, SIG_IGN);
	struct event *term = evsignal_new(base, SIGTERM, sigterm_cb, event_self_cbarg());
	assert(term != NULL);
	struct event *segv = evsignal_new(base, SIGSEGV, sigsegv_cb, event_self_cbarg());
	assert(segv!= NULL);
}


NioEventLoop::NioEventLoop()
	:index_(loopIndex++)
	,threadId_(CurrentThread::tid())
	,isMainLoop_(CurrentThread::isMainThread())
	,base_(NULL)
	,ownerGroup_(NULL)
	, timeSeq_(0)
	,loadfactor_(0)
{
	if (!setupReactorContext)
	{
		changeLimit();
		int succeed = evthread_use_pthreads();
		assert(succeed == 0);
		setupReactorContext = true;
	}
	if (t_loopInThisThread!=NULL)
	{
		std::cout << "another eventloop" << t_loopInThisThread << " is in this thread " << thread_ << std::endl;
		abort();
	}
	else
	{
		base_ = event_base_new();
		assert(base_ != NULL);
		if (index_==0)
		{
			setupSignalHandlers(base_);
		}
		t_loopInThisThread = this;
	}
}


NioEventLoop::~NioEventLoop()
{
	std::_Count_pr << "eventloop of thread " << threadId_ << " destructed in thread " << CurrentThread::tid() << std::endl;
	event_base_free(base_);
	t_loopInThisThread = NULL;
}

void NioEventLoop::loop()
{
	event_base_loop(base_, EVENT_NO_EXIT_ON_EMPTY);
	std::_Count_pr << "eventloop of thread " << threadId_ << " start looping..." << std::endl;
}

void NioEventLoop::shutdownGracefully(Timestamp time)
{
	struct timeval* tv = NULL;
	if (time.valid())
	{
		tv = const_cast<struct timeval*>((&time.toTimeval()));
	}
	event_base_loopexit(base_, tv);
	std::cout << "eventloop of thread " << threadId_ << " stop after " << time.secondsFromUnixEpoch() << " seconds..." << std::endl;
}

Timestamp NioEventLoop::pollReturnTime()
{
	struct timeval tv;
	if (event_base_gettimeofday_cached(base_, &tv) != 0)
	{
		evutil_gettimeofday(&tv, NULL);//����������ȡϵͳʱ��
	}
	return Timestamp(tv);
}

void NioEventLoop::execute(const NioTask& task)
{
	if (inEventLoop())
	{
		task();
	}
	else
	{
		Functor* fn = new Functor(task);
		event_base_once(base_, -1, EV_TIMEOUT, &Functor::execute, reinterpret_cast<void*>(fn), NULL);
	}
}

void NioEventLoop::execute(NioTask&& task)
{
	if (inEventLoop())
	{
		task();
	}
	else
	{
		Functor* fn = new Functor(std::move(task));
		event_base_once(base_, -1, EV_TIMEOUT, &Functor::execute, reinterpret_cast<void*>(fn), NULL);
	}
}

void NioEventLoop::scheduleOnce(const NioTask& task, double defferdTime)
{
	Functor* fn = new Functor(task);
	event_base_once(base_, -1, EV_TIMEOUT, &Functor::execute, reinterpret_cast<void*>(fn), &(Timestamp(defferdTime*Timestamp::kMicroSecondsPerSecond).toTimeval()));
}

void NioEventLoop::schedualOnce(NioTask&& task, double defferedTime)
{
	Functor* fn = new Functor(std::move(task));
	event_base_once(base_, -1, EV_TIMEOUT, &Functor::execute, reinterpret_cast<void*>(fn), &(Timestamp(defferdTime*Timestamp::kMicroSecondsPerSecond).toTimeval()));
}

TimerId NioEventLoop::schedualAt(const NioTask& task, Timestamp time)
{
	Functor* fn = new Functor(task);
	struct event* ev = event_new(base_, -1, EV_TIMEOUT, &Functor::execute, reinterpret_cast<void*>(fn));
	assert(ev != NULL);
	event_add(ev, &(Timestamp(timeDiff(time, Timestamp::now())).toTimeval()));

	return TimerId(timeSeq_++, ev);
}

TimerId NioEventLoop::schedualAfter(const NioTask& task, double delay)
{
	Functor* fn = new Functor(task);
	struct event* ev = event_new(base_, -1, EV_TIMEOUT, &Functor::execute, reinterpret_cast<void*>(fn));
	assert(ev != NULL);
	event_add(ev, &(Timestamp(delay*Timestamp::kMicroSecondsPerSecond).toTimeval()));

	return TimerId(timeSeq_++, ev);
}

TimerId NioEventLoop::schedualEvery(const NioTask& task, double interval)
{
	Functor* fn = new Functor(task);
	struct event* ev = event_new(base_, -1, EV_PERSIST, &Functor::execute, reinterpret_cast<void*>(fn));
	assert(ev != NULL);
	event_add(ev, &(Timestamp(interval*Timestamp::kMicroSecondsPerSecond).toTimeval()));

	return TimerId(timeSeq_++, ev);
}

TimerId NioEventLoop::schedualAt(const NioTask&& task, Timestamp time)
{
	Functor* fn = new Functor(std::move(task));
	struct event* ev = event_new(base_, -1, EV_TIMEOUT, &Functor::execute, reinterpret_cast<void*>(fn));
	assert(ev != NULL);
	event_add(ev, &(Timestamp(timeDiff(time, Timestamp::now())).toTimeval()));

	return TimerId(timeSeq_++, ev);
}

TimerId NioEventLoop::schedualAfter(const NioTask&& task, double delay)
{
	Functor* fn = new Functor(std::move(task));
	struct event* ev = event_new(base_, -1, EV_TIMEOUT, &Functor::execute, reinterpret_cast<void*>(fn));
	assert(ev != NULL);
	event_add(ev, &(Timestamp(delay*Timestamp::kMicroSecondsPerSecond).toTimeval()));

	return TimerId(timeSeq_++, ev);
}

TimerId NioEventLoop::schedualEvery(const NioTask&& task, double interval)
{
	Functor* fn = new Functor(std::move(task));
	struct event* ev = event_new(base_, -1, EV_PERSIST, &Functor::execute, reinterpret_cast<void*>(fn));
	assert(ev != NULL);
	event_add(ev, &(Timestamp(interval*Timestamp::kMicroSecondsPerSecond).toTimeval()));

	return TimerId(timeSeq_++, ev);
}

void NioEventLoop::cancle(TimerId id)
{
	id.cancel();
}

size_t NioEventLoop::rebalance(LoadOption option)
{
	switch (option)
	{
	case NioEventLoop::kAdd:
		++loadfactor_;
		break;
	case NioEventLoop::kDelete:
		--loadfactor_;
		break;
	case NioEventLoop::kReadOnly:
	default:
		break;
	}
	std::cout << "after rebalance " << "(eventloop in thread " << threadId_ << ")" << " load factor is " << loadfactor_ << std::endl;
	return loadfactor_;
}

NioEventLoopGroup* NioEventLoop::owerGroup(NioEventLoopGroup* group)
{
	assert(group != NULL);
	ownerGroup_ = group;
	return ownerGroup_;
}


NioEventLoop* NioEventLoop::getEventLoopOfCurrentThread()
{
	return t_loopInThisThread;
}


void NioEventLoop::abortNotInLoopThread()
{
	std::cout << "eventloop(" << this << ") was create in thread " << threadId_ << ",current in thread " << CurrentThread::tid() << std::endl;
}