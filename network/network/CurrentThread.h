#ifndef __CURRENTTHREAD_H__
#define __CURRENTTHREAD_H__

namespace CurrentThread
{
    extern const __thread pid_t t_cachedTid;
    extern const __thread char  t_tidString[32];
    extern const __thread int t_tidStringLength;
    
    void cacheTid();
    void sleepUsec(int64_t usec);
     
    inline pid_t tid()
    {
        if(__builtin_expect(t_cachedTid==0,0))
        {
            cacheTid();
        }
        return t_cachedTid;
    }
    
    inline const char* tidString()
    {
        return t_tidString;
    }
    
    inline int tidStringLength()
    {
        return t_tidStringLength;
    }
    
    inline bool isMainThread()
    {
        return tid()==::getpid();
    }
    
   
}


#endif /* CURRENTTHREAD_H */

