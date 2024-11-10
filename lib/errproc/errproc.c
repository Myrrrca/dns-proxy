#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <stdint.h>
#include <netinet/in.h>
#include <errno.h> 
#include <string.h>

#include "errproc.h"

int Socket(int domain, int type, int protocol)
{
  int res = socket(domain, type, protocol);
  if (res == -1) {
    perror("creation of socket failed");
    exit(EXIT_FAILURE);
  }
  return res;
}

int Setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen)
{
  int res = setsockopt(sockfd, level, optname, optval, optlen);
  if (res == -1) {
    perror("setsockopt failed");
    exit(EXIT_FAILURE);
  }
  return res;
}

uint16_t Htons(uint16_t hostshort) {
  uint16_t res = htons(hostshort);
  return res;
}

int Bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
  int res = bind(sockfd, addr, addrlen);
  if (res == -1) {
    perror("bind failed");
    exit(EXIT_FAILURE); 
  }
  return res;
}

int Listen(int sockfd, int backlog)
{
  int res = listen(sockfd, backlog);
  if (res == -1) {
    perror("listen failed");
    exit(EXIT_FAILURE); 
  }
  return res;
}

int Accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
  int res = accept(sockfd, addr, addrlen);
  if (res == -1) {
    perror("accept failed");
    exit(EXIT_FAILURE); 
  }
  return res;
}

ssize_t Recv(int sockfd, void* buf, size_t len, int flags) 
{
  int res = recv(sockfd, &buf, len, flags);
  if (res == -1) {
    perror("recv failed");
    exit(EXIT_FAILURE); 
  }
  return res;
}

ssize_t Recvfrom(int sockfd, void* buf, size_t len, int flags, 
                 struct sockaddr* src_addr, socklen_t* addrlen) 
{
  int res = recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
  if (res == -1) {
    perror("recvfrom failed");
    exit(EXIT_FAILURE); 
  }
  return res;
}

int Inet_aton(const char* cp, struct in_addr* inp)
{
  int res = inet_aton(cp, inp);
  if (res == 0) {
    fprintf(stderr, "inet_aton failed: address is not valid (%s)\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  return res;
}

char* Inet_ntoa(struct in_addr in) {
    char* ip_str = inet_ntoa(in);

    if (ip_str == NULL) {
        fprintf(stderr, "inet_ntoa failed: error converting ip address (%s)\n", 
                strerror(errno));
        exit(EXIT_FAILURE); 
    }

    return ip_str;
}

void Connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
  int res = connect(sockfd, addr, addrlen);
  if (res == -1) {
    perror("connect failed");
    exit(EXIT_FAILURE); 
  }
}

void Inet_pton(int af, const char* src, void* dst)
{
  int res = inet_pton(af, src, dst);
  if (res == 0) {
    printf("inet_pton failed: src does not contain a character "
           "string representing a valid network address in the "
           "specified adress family\n");
    exit(EXIT_FAILURE);
  }
  if (res == -1) {
    perror("inet_pton failed");
    exit(EXIT_FAILURE); 
  }
}

uint16_t Ntohs(uint16_t netshort) {
  uint16_t res = ntohs(netshort);

  if (res == 0 && netshort != 0) {
      perror("ntohs failed");
      exit(EXIT_FAILURE);
  }
  
  return res;
}

int Poll(struct pollfd* fds, nfds_t nfds, int timeout)
{
  int res = poll(fds, nfds, timeout);
  if (res == 0) {
    printf("poll failed: system call timed out before any file "
           "descriptors became ready\n");
    exit(EXIT_FAILURE);
  }
  if (res == -1) {
    perror("poll failed");
    exit(EXIT_FAILURE);
  }
  return res;
}

ssize_t Read(int fd, void* buff, size_t count)
{
  ssize_t res = read(fd, buff, count);
  if (res == 0) {
    printf("END OF FILE occured\n");
    exit(EXIT_FAILURE);
  }
  if (res == -1) {
    perror("read failed");
    exit(EXIT_FAILURE);
  }
  return res;
}

ssize_t Send(int sockfd, const void* buff, size_t len, int flags) 
{
  ssize_t res = send(sockfd, buff, len, flags);
  if (res != len) {
    printf("send warning: bytes_sent != len of original message\n");
  }
  if (res == -1) {
    perror("send failed");
    exit(EXIT_FAILURE); 
  }
  return res;
}

ssize_t Sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen) {

    ssize_t res = sendto(sockfd, buf, len, flags, dest_addr, addrlen);
    if (res == -1) {
        perror("sendto");
        exit(EXIT_FAILURE);
    }

    return res;
}

int Select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout)
{
  int res = select(nfds, readfds, writefds, exceptfds, timeout);
  if (res == -1) {
    perror("select failed");
    exit(EXIT_FAILURE);
  }
  return res;
}
