#ifndef __NIOEVENTLOOPTEST_H__
#define __NIOEVENTLOOPTEST_H__
#include <iostream>
#include "../NioEventLoop.h"
#include "../TimerId.h"
#include "../Timestamp.h"
#include "boost/bind.hpp"

class NioEventLoopTest
{
public:
	void nioEventLoopTestMain()
	{
		eventLoop.schedualAt(boost::bind(&NioEventLoopTest::print,this),addTime(Timestamp::now(),3.0));
		TimerId id1 = eventLoop.schedualAfter(boost::bind(&NioEventLoopTest::printAfter,this), 1.0);
		TimerId id2 = eventLoop.schedualEvery(boost::bind(&NioEventLoopTest::printEvery,this), 5.0);
		
                eventLoop.schedualAfter(boost::bind(&NioEventLoopTest::cancel,this,id2),25.0);
                
                eventLoop.scheduleOnce(boost::bind(&NioEventLoopTest::print,this),3.0);
		eventLoop.loop();
	}

private:
	void print()
	{
		std::cout << __FUNCTION__ << std::endl;
	}

	void printAfter()
	{
		std::cout << __FUNCTION__ << std::endl;
	}

	void printEvery()
	{
		std::cout << __FUNCTION__ << std::endl;
	}

	void cancel(TimerId id)
	{
            	std::cout << "cancel(TimerId id)" << std::endl;
		id.cancel();
	}
        
private:
   NioEventLoop eventLoop;
};




#endif


