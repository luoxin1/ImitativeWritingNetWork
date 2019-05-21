#include"CurrentThread.h"

__thread pid_t t_cachedTid;
__thread char  t_tidString[32];
__thread int t_tidStringLength;

void CurrentThread::cacheTid()
{
    if(t_cachedTid==0)
    {
        t_cachedTid=static_cast<pid_t>(syscall(SYS_gettid));
        t_tidStringLength=snprintf(t_tidString,sizeof(t_tidString),"%5d",t_cachedTid);
    }
}

void CurrentThread::sleepUsec(int64_t usec)
{
    struct timespec ts={0,0};
    ts.tv_sec=static_cast<time_t>(usec/1000000);
    ts.tv_nsec=static_cast<time_t>(usec%1000000000);
    nanosleep(&ts,NULL);
}
