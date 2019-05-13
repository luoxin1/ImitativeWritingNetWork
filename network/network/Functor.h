#ifndef __FUNCTOR_H__
#define __FUNCTOR_H__
#include"boost/noncopyable.hpp"
#include"boost/function.hpp"
#include"event2/util.h"

class Functor:boost::noncopyable
{
	typedef boost::function<void()> InternalCallback;

	Functor(const InternalCallback& callback, bool persist = false);
	Functor(InternalCallback&& callback, bool persist = false);
	~Functor();

	static void excute(evutil_socket_t fd, short what, void* privdata);

private:
	Functor(){}

private:
	bool persist_;
	InternalCallback callback_;
};

#endif // !__FUNCTOR_H__
