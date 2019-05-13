#include"Functor.h"

Functor::Functor(const InternalCallback& callback, bool persist)
	:persist_(persist)
	,callback_(callback)
{

}

Functor::Functor(InternalCallback&& callback, bool persist)
	:persist_(persist)
	, callback_(std::move(callback))
{

}

Functor::~Functor()
{

}

void Functor::excute(evutil_socket_t fd, short what, void* privdata)
{
	Functor* fn = reinterpret_cast<Functor*>(privdata);
	fn->callback_();
	if (!fn->persist_)
	{
		delete fn;
	}
}
