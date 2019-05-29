#ifndef __SOCKETSOPS_H__
#define __SOCKETSOPS_H__
#include <arpa/inet.h>

namespace sockets
{
    const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);

    const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
    
    struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr);

    const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);

    const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr* addr);

    void setOption(int fd, int opt, int level, int optval);

    void shutdownWrite(int sockfd);

    void toIpPort(char* buf, size_t size, const struct sockaddr* addr);

    void toIp(char* buf, size_t size, const struct sockaddr* addr);

    void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);

    void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr);

    struct sockaddr_in6 getLocalAddress(int sockfd);

    struct sockaddr_in6 getRemoteAddress(int sockfd);

    bool isSelfConnect(int sockfd);
}

#endif