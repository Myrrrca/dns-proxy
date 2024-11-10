#pragma once 

#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <stdint.h>
#include <netinet/in.h>
#include <sys/select.h>

int Socket(int domain, int type, int protocol);

int Setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen);

uint16_t Htons(uint16_t hostshort);

uint16_t Ntohs(uint16_t netshort);

int Bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen);

int Listen(int sockfd, int backlog);

int Accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen);

ssize_t Recv(int sockfd, void* buf, size_t len, int flags);

ssize_t Recvfrom(int sockfd, void* buf, size_t len, int flags, 
                 struct sockaddr* src_addr, socklen_t* addrlen); 

int Inet_aton(const char* cp, struct in_addr* inp);

char* Inet_ntoa(struct in_addr in);

void Connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen);

void Inet_pton(int af, const char* src, void* dst);

int Poll(struct pollfd* fds, nfds_t nfds, int timeout);

ssize_t Read(int fd, void* buff, size_t count);

ssize_t Send(int sockfd, const void* buff, size_t len, int flags);

ssize_t Sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);

int Select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout);
