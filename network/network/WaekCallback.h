#ifndef __WAEKCALLBACK_H__
#define __WAEKCALLBACK_H__
#include "boost/function.hpp"
#include "boost/smart_ptr.hpp"

template <typename CLASS,typename...ARGS>
class WeakCallback
{
public:
	WeakCallback(const boost::weak_ptr<CLASS>& object, const boost::function<void(CLASS*, ARGS...)>& function)
		:object_(object_)
		, function_(function)
	{

	}

	void operator()(ARGS&& ... args) const
	{
		boost::shared_ptr<CLASS> ptr (object_.lock());
		if (ptr)
		{
			function_(ptr.get(), std::forward<ARGS>(args)...);
		}
	}

private:
	boost::weak_ptr<CLASS> object_;
	boost::function<void(CLASS*, ARGS...)> function_;
};

template<typename CLASS,typename...ARGS>
WeakCallback<CLASS, ARGS...> makeWeakCallback(const boost::shared_ptr<CLASS>& object, void(CLASS::*function)(ARGS...))
{
	return WeakCallback< CLASS, ARGS...>(object, function);
}

template<typename CLASS, typename...ARGS>
WeakCallback<CLASS, ARGS...> makeWeakCallback(const boost::shared_ptr<CLASS>& object, void(CLASS::*function)(ARGS...) const)
{
	return WeakCallback< CLASS, ARGS...>(object, function);
}

#endif
