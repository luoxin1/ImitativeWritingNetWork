#ifndef __TIMERID_H__
#define __TIMERID_H__

class Functor;

class TimerId
{
public:
    TimerId()
    :sequence_(0)
    ,event_(NULL)
    ,privdata_(NULL)
    {        
        
    }
	TimerId(size_t sequence,struct event* e,Functor* privdata=NULL)
		:sequence_()
		,event_(e)
		,privdata_(privdata)
	{

	}

	bool valid() const { return event_ != NULL; }

	void cancel();

private:
	size_t sequence_;
	struct event* event_;
	Functor* privdata_;
};

#endif