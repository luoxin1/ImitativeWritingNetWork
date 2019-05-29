#ifndef __SOCKET_H__
#define __SOCKET_H__
#include"event2/util.h"
#include "boost/noncopyable.hpp"

class Socket:boost::noncopyable
{
public:
    explicit Socket(evutil_socket_t sockfd)
    :sockfd_(sockfd)
    {

    }

    ~Socket();
    
    evutil_socket_t fd() const
    {
        return sockfd_;
    }
    
    bool getTcpInfo(struct tcp_info* info) const;    
    bool getTcpInfoString(char* buf,int len) const;

    void shutdownWrite();
    void option(int opt,int level,int optval);
    
private:
    const evutil_socket_t sockfd_;
            
};

#endif /* SOCKET_H */

