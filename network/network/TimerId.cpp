#include "TimerId.h"
#include<event.h>

void TimerId::cancel()
{
	if (event_!=NULL)
	{
		sequence_ = 0;
		event_del(event_);
		event_free(event_);
		event_ = NULL;
	}
	if (privdata_ != NULL)
	{
		delete privdata_;
		privdata_ = NULL;
	}
}