#include"Socket.h"
#include"SocketsOps.h"
#include<netinet/tcp.h>
#include<netinet/in.h>
#include<memory.h>
#include<stdio.h>

Socket::~Socket()
{
    evutil_closesocket(sockfd_);
}
    
bool Socket::getTcpInfo(struct tcp_info* info) const
{
    socklen_t len = sizeof(*info);
    memset(info,0,len);
    return getsockopt(sockfd_,SOL_TCP,TCP_INFO,info,&len);
}

bool Socket::getTcpInfoString(char* buf,int len) const
{
    struct tcp_info tcpi;
    bool res = getTcpInfo(&tcpi);
    if(res)
    {
        snprintf(buf, len, "unrecovered=%u "
			"rto=%u ato=%u snd_mss=%u rcv_mss=%u "
			"lost=%u retrans=%u rtt=%u rttvar=%u "
			"sshthresh=%u cwnd=%u total_retrans=%u",
			tcpi.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
			tcpi.tcpi_rto,          // Retransmit timeout in usec
			tcpi.tcpi_ato,          // Predicted tick of soft clock in usec
			tcpi.tcpi_snd_mss,
			tcpi.tcpi_rcv_mss,
			tcpi.tcpi_lost,         // Lost packets
			tcpi.tcpi_retrans,      // Retransmitted packets out
			tcpi.tcpi_rtt,          // Smoothed round trip time in usec
			tcpi.tcpi_rttvar,       // Medium deviation
			tcpi.tcpi_snd_ssthresh,
			tcpi.tcpi_snd_cwnd,
			tcpi.tcpi_total_retrans);  // Total retransmits for entire connection
    }
    return res;
}

void Socket::shutdownWrite()
{
    sockets::shutdownWrite(static_cast<int>(sockfd_));
}

void Socket::option(int opt,int level,int optval)
{
    sockets::setOption(static_cast<int>(sockfd_),opt,level,optval);
}
    
